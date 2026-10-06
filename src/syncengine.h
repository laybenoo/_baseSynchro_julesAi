#ifndef SYNCENGINE_H
#define SYNCENGINE_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QQueue>
#include "databasemanager.h"

class SyncEngine : public QObject {
    Q_OBJECT
public:
    explicit SyncEngine(DatabaseManager *dbMgr, QObject *parent = nullptr);

    void setBaseUrl(const QString &url) { m_baseUrl = url; }
    QString baseUrl() const { return m_baseUrl; }

    void syncTable(const QString &tableName);
    void syncAllTables(const QStringList &tableNames);

signals:
    void logMessage(const QString &msg);
    void tableSyncStarted(const QString &tableName);
    void tableSyncFinished(const QString &tableName, bool success, int insertedCount, const QString &message);
    void batchProgressChanged(int processed, int total);
    void batchFinished();

private slots:
    void processNextInQueue();

private:
    DatabaseManager *m_dbMgr = nullptr;
    QNetworkAccessManager m_netMgr;
    QString m_baseUrl = "https://example.com";
    QQueue<QString> m_queue;
    int m_batchTotal = 0;
    int m_batchProcessed = 0;
    bool m_isProcessing = false;
};

#endif // SYNCENGINE_H
