#include "lottomodel.h"
#include <QDebug>

LottoModel::LottoModel(QObject *parent)
    : QAbstractListModel(parent)
{
    m_dbMgr.openDatabase("base.db");
    initTables();

    m_syncEngine.setBaseUrl(m_domain);

    connect(&m_syncEngine, &SyncEngine::logMessage, this, &LottoModel::logEmitted);

    connect(&m_syncEngine, &SyncEngine::tableSyncStarted, this, [this](const QString &tableName) {
        for (int i = 0; i < m_allTables.size(); ++i) {
            if (m_allTables[i].tableName == tableName) {
                m_allTables[i].status = "Synchronisation...";
                m_allTables[i].lastMessage = "En cours...";

                int visibleRow = m_filteredIndices.indexOf(i);
                if (visibleRow >= 0) {
                    QModelIndex idx = createIndex(visibleRow, 0);
                    emit dataChanged(idx, idx, {StatusRole, LastMessageRole});
                }
                break;
            }
        }
        emit countsChanged();
    });

    connect(&m_syncEngine, &SyncEngine::tableSyncFinished, this,
            [this](const QString &tableName, bool success, int insertedCount, const QString &message) {
        for (int i = 0; i < m_allTables.size(); ++i) {
            if (m_allTables[i].tableName == tableName) {
                m_allTables[i].maxDraw = m_dbMgr.getMaxLocalDraw(tableName);
                if (success) {
                    if (insertedCount > 0) {
                        m_allTables[i].status = "Nouveaux tirages";
                        m_allTables[i].newDrawsCount = insertedCount;
                    } else {
                        m_allTables[i].status = "À jour";
                        m_allTables[i].newDrawsCount = 0;
                    }
                    m_allTables[i].lastMessage = message;
                } else {
                    m_allTables[i].status = "Erreur";
                    m_allTables[i].lastMessage = message;
                }

                int visibleRow = m_filteredIndices.indexOf(i);
                if (visibleRow >= 0) {
                    QModelIndex idx = createIndex(visibleRow, 0);
                    emit dataChanged(idx, idx, {MaxDrawRole, StatusRole, NewDrawsCountRole, LastMessageRole});
                }
                break;
            }
        }
        emit countsChanged();
    });

    connect(&m_syncEngine, &SyncEngine::batchProgressChanged, this, [this](int processed, int total) {
        m_processedCount = processed;
        m_totalCount = total;
        m_isSyncing = (processed < total);
        emit progressChanged();
        emit isSyncingChanged();
    });

    connect(&m_syncEngine, &SyncEngine::batchFinished, this, [this]() {
        m_isSyncing = false;
        emit isSyncingChanged();
    });
}

void LottoModel::initTables()
{
    struct TableCat {
        QString country;
        QStringList tables;
    };

    QList<TableCat> catalog = {
        {"Togo", {"diamant", "goldTg", "cashTg", "boomTg", "benz", "prestigeTg", "millionsTg", "superTg", "kadoo", "kingTg", "sam", "bingoTg", "matinalTg"}},
        {"Ghana & National", {"fortuneLundi", "fortuneMardi", "star", "fortuneJeudi", "fortune", "star3", "star2", "digital00BJ", "digital21BJ", "mondaySpecial", "luckyTuesday", "nationalMidweek", "fortuneThursday", "fridayBonanza", "nationalWeekly", "aseda", "noon"}},
        {"Côte d'Ivoire", {"reveil", "etoile", "akwaba", "matinale", "emergence", "sika", "premiereHeure", "fortuneCi", "baraka", "kadoCi", "privilege", "monni", "cash", "solution", "wari", "soutra", "diamantCi", "moaye", "benediction", "prestige", "awale", "espoir", "digitalCI", "specialDayOffCI", "afterWork"}},
        {"Niger", {"mamakiNiger", "soudjiNiger", "arzikiNiger", "gomniNiger", "irmaSabouNiger", "farahaNiger", "guimbaNiger"}},
        {"Italie", {"malta", "bari", "cagliari", "firenze", "genova", "milano", "napoli", "roma", "nazionale", "palermo", "torino", "venezia"}}
    };

    m_allTables.clear();
    for (const auto &cat : catalog) {
        for (const auto &tbl : cat.tables) {
            LottoTableInfo info;
            info.tableName = tbl;
            info.country = cat.country;
            info.maxDraw = m_dbMgr.getMaxLocalDraw(tbl);
            info.status = "À jour";
            info.lastMessage = "Prêt";
            m_allTables.append(info);
        }
    }

    updateFilteredIndices();
    emit totalTablesCountChanged();
    emit countsChanged();
}

void LottoModel::updateFilteredIndices()
{
    beginResetModel();
    m_filteredIndices.clear();

    for (int i = 0; i < m_allTables.size(); ++i) {
        bool matchCountry = (m_filterCountry == "Tous" || m_allTables[i].country == m_filterCountry);
        bool matchSearch = m_searchFilter.isEmpty() ||
                           m_allTables[i].tableName.contains(m_searchFilter, Qt::CaseInsensitive) ||
                           m_allTables[i].country.contains(m_searchFilter, Qt::CaseInsensitive);

        if (matchCountry && matchSearch) {
            m_filteredIndices.append(i);
        }
    }
    endResetModel();
}

void LottoModel::setDomain(const QString &domain)
{
    if (m_domain != domain) {
        m_domain = domain;
        m_syncEngine.setBaseUrl(m_domain);
        emit domainChanged();
    }
}

void LottoModel::setFilterCountry(const QString &country)
{
    if (m_filterCountry != country) {
        m_filterCountry = country;
        updateFilteredIndices();
        emit filterCountryChanged();
    }
}

void LottoModel::setSearchFilter(const QString &search)
{
    if (m_searchFilter != search) {
        m_searchFilter = search;
        updateFilteredIndices();
        emit searchFilterChanged();
    }
}

int LottoModel::upToDateCount() const
{
    int cnt = 0;
    for (const auto &tbl : m_allTables) {
        if (tbl.status == "À jour") cnt++;
    }
    return cnt;
}

int LottoModel::updatedCount() const
{
    int cnt = 0;
    for (const auto &tbl : m_allTables) {
        if (tbl.status == "Nouveaux tirages") cnt++;
    }
    return cnt;
}

int LottoModel::errorCount() const
{
    int cnt = 0;
    for (const auto &tbl : m_allTables) {
        if (tbl.status == "Erreur") cnt++;
    }
    return cnt;
}

int LottoModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return m_filteredIndices.size();
}

QVariant LottoModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_filteredIndices.size())
        return QVariant();

    int actualIndex = m_filteredIndices[index.row()];
    const LottoTableInfo &info = m_allTables[actualIndex];

    switch (role) {
    case TableNameRole: return info.tableName;
    case CountryRole: return info.country;
    case MaxDrawRole: return info.maxDraw;
    case StatusRole: return info.status;
    case NewDrawsCountRole: return info.newDrawsCount;
    case LastMessageRole: return info.lastMessage;
    default: return QVariant();
    }
}

QHash<int, QByteArray> LottoModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[TableNameRole] = "tableName";
    roles[CountryRole] = "country";
    roles[MaxDrawRole] = "maxDraw";
    roles[StatusRole] = "status";
    roles[NewDrawsCountRole] = "newDrawsCount";
    roles[LastMessageRole] = "lastMessage";
    return roles;
}

void LottoModel::syncTable(int visibleIndex)
{
    if (visibleIndex < 0 || visibleIndex >= m_filteredIndices.size()) return;
    int actualIndex = m_filteredIndices[visibleIndex];
    m_syncEngine.syncTable(m_allTables[actualIndex].tableName);
}

void LottoModel::syncAll()
{
    QStringList tables;
    for (const auto &info : m_allTables) {
        tables.append(info.tableName);
    }
    m_syncEngine.syncAllTables(tables);
}

void LottoModel::refreshLocalDraws()
{
    beginResetModel();
    for (auto &info : m_allTables) {
        info.maxDraw = m_dbMgr.getMaxLocalDraw(info.tableName);
    }
    endResetModel();
    emit countsChanged();
}
