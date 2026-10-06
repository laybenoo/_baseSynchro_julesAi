#include "databasemanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
{
}

DatabaseManager::~DatabaseManager()
{
    closeDatabase();
}

bool DatabaseManager::openDatabase(const QString &dbPath)
{
    if (QSqlDatabase::contains("lotto_connection")) {
        m_db = QSqlDatabase::database("lotto_connection");
    } else {
        m_db = QSqlDatabase::addDatabase("QSQLITE", "lotto_connection");
    }
    m_db.setDatabaseName(dbPath);

    if (!m_db.open()) {
        qWarning() << "Failed to open database:" << m_db.lastError().text();
        return false;
    }
    return true;
}

void DatabaseManager::closeDatabase()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
}

bool DatabaseManager::ensureTableExists(const QString &tableName)
{
    if (!m_db.isOpen() && !openDatabase()) return false;

    QString queryStr = QString(
        "CREATE TABLE IF NOT EXISTS %1 ("
        "tirage INTEGER PRIMARY KEY, "
        "jour INTEGER, "
        "mois INTEGER, "
        "annee INTEGER, "
        "d1 INTEGER, "
        "d2 INTEGER, "
        "d3 INTEGER, "
        "d4 INTEGER, "
        "d5 INTEGER, "
        "total INTEGER"
        ")"
    ).arg(tableName);

    QSqlQuery q(m_db);
    if (!q.exec(queryStr)) {
        qWarning() << "Error creating table" << tableName << ":" << q.lastError().text();
        return false;
    }
    return true;
}

int DatabaseManager::getMaxLocalDraw(const QString &tableName)
{
    if (!ensureTableExists(tableName)) return 0;

    QString queryStr = QString("SELECT MAX(tirage) FROM %1").arg(tableName);
    QSqlQuery q(m_db);
    if (q.exec(queryStr) && q.next()) {
        bool ok = false;
        int maxVal = q.value(0).toInt(&ok);
        if (ok) return maxVal;
    }
    return 0;
}

bool DatabaseManager::insertDraws(const QString &tableName, const QList<DrawRecord> &records)
{
    if (records.isEmpty()) return true;
    if (!ensureTableExists(tableName)) return false;

    if (!m_db.transaction()) {
        qWarning() << "Failed to begin transaction:" << m_db.lastError().text();
        return false;
    }

    QString queryStr = QString(
        "INSERT OR REPLACE INTO %1 (tirage, jour, mois, annee, d1, d2, d3, d4, d5, total) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"
    ).arg(tableName);

    QSqlQuery q(m_db);
    q.prepare(queryStr);

    for (const auto &rec : records) {
        q.addBindValue(rec.tirage);
        q.addBindValue(rec.jour);
        q.addBindValue(rec.mois);
        q.addBindValue(rec.annee);
        q.addBindValue(rec.d1);
        q.addBindValue(rec.d2);
        q.addBindValue(rec.d3);
        q.addBindValue(rec.d4);
        q.addBindValue(rec.d5);
        q.addBindValue(rec.total);

        if (!q.exec()) {
            qWarning() << "Error inserting record in" << tableName << ":" << q.lastError().text();
            m_db.rollback();
            return false;
        }
    }

    if (!m_db.commit()) {
        qWarning() << "Failed to commit transaction:" << m_db.lastError().text();
        m_db.rollback();
        return false;
    }

    return true;
}
