#ifndef LOTTOMODEL_H
#define LOTTOMODEL_H

#include <QAbstractListModel>
#include <QList>
#include <QStringList>
#include "databasemanager.h"
#include "syncengine.h"

struct LottoTableInfo {
    QString tableName;
    QString country;
    int maxDraw = 0;
    QString status = "Initial"; // "À jour", "Nouveaux tirages", "Synchronisation...", "Erreur"
    int newDrawsCount = 0;
    QString lastMessage = "";
};

class LottoModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(QString domain READ domain WRITE setDomain NOTIFY domainChanged)
    Q_PROPERTY(int processedCount READ processedCount NOTIFY progressChanged)
    Q_PROPERTY(int totalCount READ totalCount NOTIFY progressChanged)
    Q_PROPERTY(double progressPercentage READ progressPercentage NOTIFY progressChanged)
    Q_PROPERTY(bool isSyncing READ isSyncing NOTIFY isSyncingChanged)
    Q_PROPERTY(QString filterCountry READ filterCountry WRITE setFilterCountry NOTIFY filterCountryChanged)
    Q_PROPERTY(QString searchFilter READ searchFilter WRITE setSearchFilter NOTIFY searchFilterChanged)
    Q_PROPERTY(int totalTablesCount READ totalTablesCount NOTIFY totalTablesCountChanged)
    Q_PROPERTY(int upToDateCount READ upToDateCount NOTIFY countsChanged)
    Q_PROPERTY(int updatedCount READ updatedCount NOTIFY countsChanged)
    Q_PROPERTY(int errorCount READ errorCount NOTIFY countsChanged)

public:
    enum LottoRoles {
        TableNameRole = Qt::UserRole + 1,
        CountryRole,
        MaxDrawRole,
        StatusRole,
        NewDrawsCountRole,
        LastMessageRole
    };

    explicit LottoModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString domain() const { return m_domain; }
    void setDomain(const QString &domain);

    int processedCount() const { return m_processedCount; }
    int totalCount() const { return m_totalCount; }
    double progressPercentage() const { return m_totalCount > 0 ? (double)m_processedCount / m_totalCount : 0.0; }
    bool isSyncing() const { return m_isSyncing; }

    QString filterCountry() const { return m_filterCountry; }
    void setFilterCountry(const QString &country);

    QString searchFilter() const { return m_searchFilter; }
    void setSearchFilter(const QString &search);

    int totalTablesCount() const { return m_allTables.size(); }
    int upToDateCount() const;
    int updatedCount() const;
    int errorCount() const;

    Q_INVOKABLE void syncTable(int index);
    Q_INVOKABLE void syncAll();
    Q_INVOKABLE void refreshLocalDraws();

signals:
    void domainChanged();
    void progressChanged();
    void isSyncingChanged();
    void filterCountryChanged();
    void searchFilterChanged();
    void totalTablesCountChanged();
    void countsChanged();
    void logEmitted(const QString &message);

private:
    void initTables();
    void updateFilteredIndices();

    DatabaseManager m_dbMgr;
    SyncEngine m_syncEngine{&m_dbMgr, this};

    QList<LottoTableInfo> m_allTables;
    QList<int> m_filteredIndices;

    QString m_domain = "https://lotto-api.example.com";
    QString m_filterCountry = "Tous";
    QString m_searchFilter = "";

    int m_processedCount = 0;
    int m_totalCount = 0;
    bool m_isSyncing = false;
};

#endif // LOTTOMODEL_H
