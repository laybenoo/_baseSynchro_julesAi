#include "syncengine.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QUrlQuery>
#include <QDebug>

static int jsonToInt(const QJsonValue &val)
{
    if (val.isDouble()) return val.toInt();
    if (val.isString()) return val.toString().toInt();
    return val.toVariant().toInt();
}

SyncEngine::SyncEngine(DatabaseManager *dbMgr, QObject *parent)
    : QObject(parent), m_dbMgr(dbMgr)
{
}

void SyncEngine::syncTable(const QString &tableName)
{
    if (!m_queue.contains(tableName)) {
        m_queue.enqueue(tableName);
    }
    if (!m_isProcessing) {
        m_batchTotal = m_queue.size();
        m_batchProcessed = 0;
        processNextInQueue();
    }
}

void SyncEngine::syncAllTables(const QStringList &tableNames)
{
    for (const QString &tbl : tableNames) {
        if (!m_queue.contains(tbl)) {
            m_queue.enqueue(tbl);
        }
    }
    if (!m_isProcessing) {
        m_batchTotal = m_queue.size();
        m_batchProcessed = 0;
        processNextInQueue();
    }
}

void SyncEngine::processNextInQueue()
{
    if (m_queue.isEmpty()) {
        m_isProcessing = false;
        emit batchFinished();
        return;
    }

    m_isProcessing = true;
    QString tableName = m_queue.dequeue();

    emit tableSyncStarted(tableName);
    emit logMessage(QString("[%1] Début de la synchronisation...").arg(tableName));

    int maxLocalDraw = m_dbMgr->getMaxLocalDraw(tableName);
    emit logMessage(QString("[%1] Dernier tirage local: %2").arg(tableName).arg(maxLocalDraw));

    QString baseUrlStr = m_baseUrl.trimmed();
    if (!baseUrlStr.startsWith("http://") && !baseUrlStr.startsWith("https://")) {
        baseUrlStr = "https://" + baseUrlStr;
    }
    if (!baseUrlStr.endsWith("/")) {
        baseUrlStr += "/";
    }
    QString fullUrlStr = baseUrlStr + QString("api/tMobile/fetchlotto.php?lotto=%1").arg(tableName);

    QUrl targetUrl(fullUrlStr);
    QNetworkRequest request(targetUrl);
    QNetworkReply *reply = m_netMgr.get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply, tableName, maxLocalDraw]() {
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            QString errMsg = reply->errorString();
            emit logMessage(QString("[%1] ERREUR Réseau: %2").arg(tableName, errMsg));
            emit tableSyncFinished(tableName, false, 0, QString("Erreur réseau: %1").arg(errMsg));

            m_batchProcessed++;
            emit batchProgressChanged(m_batchProcessed, m_batchTotal);
            processNextInQueue();
            return;
        }

        QByteArray data = reply->readAll();
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

        if (parseError.error != QJsonParseError::NoError || !doc.isArray()) {
            QString errMsg = parseError.errorString();
            emit logMessage(QString("[%1] ERREUR Parsing JSON: %2").arg(tableName, errMsg));
            emit tableSyncFinished(tableName, false, 0, "Format JSON invalide");

            m_batchProcessed++;
            emit batchProgressChanged(m_batchProcessed, m_batchTotal);
            processNextInQueue();
            return;
        }

        QJsonArray arr = doc.array();
        QList<DrawRecord> recordsToInsert;

        for (const QJsonValue &val : arr) {
            if (!val.isObject()) continue;
            QJsonObject obj = val.toObject();

            int draw = jsonToInt(obj.value("draw"));
            if (draw > maxLocalDraw) {
                DrawRecord rec;
                rec.tirage = draw;
                rec.jour = jsonToInt(obj.value("day"));
                rec.mois = jsonToInt(obj.value("month"));
                rec.annee = jsonToInt(obj.value("year"));
                rec.d1 = jsonToInt(obj.value("d1"));
                rec.d2 = jsonToInt(obj.value("d2"));
                rec.d3 = jsonToInt(obj.value("d3"));
                rec.d4 = jsonToInt(obj.value("d4"));
                rec.d5 = jsonToInt(obj.value("d5"));
                rec.total = jsonToInt(obj.value("total"));

                recordsToInsert.append(rec);
            }
        }

        if (recordsToInsert.isEmpty()) {
            emit logMessage(QString("[%1] À jour. Aucun nouveau tirage (JSON total: %2)").arg(tableName).arg(arr.size()));
            emit tableSyncFinished(tableName, true, 0, "À jour");
        } else {
            bool ok = m_dbMgr->insertDraws(tableName, recordsToInsert);
            if (ok) {
                emit logMessage(QString("[%1] Transaction réussie: %2 tirages ajoutés.")
                                    .arg(tableName).arg(recordsToInsert.size()));
                emit tableSyncFinished(tableName, true, recordsToInsert.size(),
                                       QString("%1 nouveaux tirages").arg(recordsToInsert.size()));
            } else {
                emit logMessage(QString("[%1] ERREUR d'insertion SQLite (Rollback effectué).").arg(tableName));
                emit tableSyncFinished(tableName, false, 0, "Erreur d'insertion BD");
            }
        }

        m_batchProcessed++;
        emit batchProgressChanged(m_batchProcessed, m_batchTotal);
        processNextInQueue();
    });
}
