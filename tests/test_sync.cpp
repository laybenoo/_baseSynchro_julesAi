#include <QtTest>
#include <QHttpServer>
#include <QHttpServerResponse>
#include <QEventLoop>
#include "../src/databasemanager.h"
#include "../src/syncengine.h"
#include "../src/lottomodel.h"

class TestSync : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testDatabaseOperations();
    void testJsonParsingAndFilteringWithMockServer();
    void testRollbackOnInvalidJson();
    void testBatchSyncQueue();
    void testLottoModelCatalogAndFiltering();

private:
    QHttpServer m_server;
    quint16 m_serverPort = 0;
};

void TestSync::initTestCase()
{
    QFile::remove("test_app_base.db");

    // Setup Mock HTTP Server
    m_server.route("/api/tMobile/fetchlotto.php", [](const QHttpServerRequest &req) {
        QUrlQuery query(req.url().query());
        QString lotto = query.queryItemValue("lotto");

        if (lotto == "error_json") {
            return QHttpServerResponse("INVALID_JSON_RESPONSE_STRING", QHttpServerResponse::StatusCode::Ok);
        }

        if (lotto == "empty") {
            return QHttpServerResponse("[]", QHttpServerResponse::StatusCode::Ok);
        }

        // Return mock lotto draws array with ignored fields (id, lotto, name, groupes, jourTirages, tiragePerDay)
        QJsonArray arr;

        QJsonObject obj1;
        obj1["id"] = 101;
        obj1["lotto"] = lotto;
        obj1["name"] = "Sample Name";
        obj1["groupes"] = "Grp A";
        obj1["jourTirages"] = "Mon";
        obj1["tiragePerDay"] = 1;
        obj1["draw"] = 890;
        obj1["day"] = 1;
        obj1["month"] = 5;
        obj1["year"] = 2026;
        obj1["d1"] = 10; obj1["d2"] = 20; obj1["d3"] = 30; obj1["d4"] = 40; obj1["d5"] = 50;
        obj1["total"] = 150;
        arr.append(obj1);

        QJsonObject obj2;
        obj2["id"] = 102;
        obj2["lotto"] = lotto;
        obj2["draw"] = 892;
        obj2["day"] = 5;
        obj2["month"] = 10;
        obj2["year"] = 2026;
        obj2["d1"] = 15; obj2["d2"] = 72; obj2["d3"] = 9; obj2["d4"] = 75; obj2["d5"] = 6;
        obj2["total"] = 177;
        arr.append(obj2);

        QJsonDocument doc(arr);
        return QHttpServerResponse(doc.toJson(), QHttpServerResponse::StatusCode::Ok);
    });

    m_serverPort = m_server.listen(QHostAddress::LocalHost, 0);
    QVERIFY(m_serverPort > 0);
}

void TestSync::cleanupTestCase()
{
    QFile::remove("test_app_base.db");
}

void TestSync::testDatabaseOperations()
{
    DatabaseManager db;
    QVERIFY(db.openDatabase("test_app_base.db"));

    // Default max draw is 0
    QCOMPARE(db.getMaxLocalDraw("diamant"), 0);

    // Insert records
    QList<DrawRecord> list;
    DrawRecord r1{50, 1, 1, 2026, 1, 2, 3, 4, 5, 15};
    DrawRecord r2{75, 2, 1, 2026, 6, 7, 8, 9, 10, 40};
    list.append(r1);
    list.append(r2);

    QVERIFY(db.insertDraws("diamant", list));
    QCOMPARE(db.getMaxLocalDraw("diamant"), 75);
}

void TestSync::testJsonParsingAndFilteringWithMockServer()
{
    DatabaseManager db;
    QVERIFY(db.openDatabase("test_app_base.db"));

    // Pre-insert draw #890 for table 'benz'
    QList<DrawRecord> initialRecords;
    initialRecords.append(DrawRecord{890, 1, 5, 2026, 10, 20, 30, 40, 50, 150});
    QVERIFY(db.insertDraws("benz", initialRecords));
    QCOMPARE(db.getMaxLocalDraw("benz"), 890);

    SyncEngine syncEngine(&db);
    syncEngine.setBaseUrl(QString("http://127.0.0.1:%1").arg(m_serverPort));

    QEventLoop loop;
    bool syncSuccess = false;
    int inserted = -1;

    connect(&syncEngine, &SyncEngine::tableSyncFinished,
            [&loop, &syncSuccess, &inserted](const QString &tbl, bool ok, int count, const QString &) {
        if (tbl == "benz") {
            syncSuccess = ok;
            inserted = count;
            loop.quit();
        }
    });

    syncEngine.syncTable("benz");
    loop.exec();

    QVERIFY(syncSuccess);
    // Draw 890 was filtered out (since 890 <= maxLocalDraw 890), so only draw 892 should be inserted!
    QCOMPARE(inserted, 1);
    QCOMPARE(db.getMaxLocalDraw("benz"), 892);
}

void TestSync::testRollbackOnInvalidJson()
{
    DatabaseManager db;
    QVERIFY(db.openDatabase("test_app_base.db"));

    SyncEngine syncEngine(&db);
    syncEngine.setBaseUrl(QString("http://127.0.0.1:%1").arg(m_serverPort));

    QEventLoop loop;
    bool syncSuccess = true;

    connect(&syncEngine, &SyncEngine::tableSyncFinished,
            [&loop, &syncSuccess](const QString &tbl, bool ok, int, const QString &) {
        if (tbl == "error_json") {
            syncSuccess = ok;
            loop.quit();
        }
    });

    syncEngine.syncTable("error_json");
    loop.exec();

    // Invalid JSON should trigger sync failure and rollback
    QVERIFY(!syncSuccess);
}

void TestSync::testBatchSyncQueue()
{
    DatabaseManager db;
    QVERIFY(db.openDatabase("test_app_base.db"));

    SyncEngine syncEngine(&db);
    syncEngine.setBaseUrl(QString("http://127.0.0.1:%1").arg(m_serverPort));

    QEventLoop loop;
    int batchProcessed = 0;
    int batchTotal = 0;

    connect(&syncEngine, &SyncEngine::batchProgressChanged,
            [&batchProcessed, &batchTotal](int processed, int total) {
        batchProcessed = processed;
        batchTotal = total;
    });

    connect(&syncEngine, &SyncEngine::batchFinished, &loop, &QEventLoop::quit);

    QStringList tablesToSync = {"kadoo", "kingTg", "sam"};
    syncEngine.syncAllTables(tablesToSync);
    loop.exec();

    QCOMPARE(batchProcessed, 3);
    QCOMPARE(batchTotal, 3);
    QCOMPARE(db.getMaxLocalDraw("kadoo"), 892);
    QCOMPARE(db.getMaxLocalDraw("kingTg"), 892);
    QCOMPARE(db.getMaxLocalDraw("sam"), 892);
}

void TestSync::testLottoModelCatalogAndFiltering()
{
    LottoModel model;

    // Total catalog table count: 13 (Togo) + 17 (Ghana) + 25 (CI) + 7 (Niger) + 12 (Italie) = 74
    QCOMPARE(model.totalTablesCount(), 74);
    QCOMPARE(model.rowCount(), 74);

    // Filter by Togo (13 tables)
    model.setFilterCountry("Togo");
    QCOMPARE(model.rowCount(), 13);

    // Filter by Ghana & National (17 tables)
    model.setFilterCountry("Ghana & National");
    QCOMPARE(model.rowCount(), 17);

    // Filter by Côte d'Ivoire (25 tables)
    model.setFilterCountry("Côte d'Ivoire");
    QCOMPARE(model.rowCount(), 25);

    // Filter by Niger (7 tables)
    model.setFilterCountry("Niger");
    QCOMPARE(model.rowCount(), 7);

    // Filter by Italie (12 tables)
    model.setFilterCountry("Italie");
    QCOMPARE(model.rowCount(), 12);

    // Reset filter to Tous
    model.setFilterCountry("Tous");
    QCOMPARE(model.rowCount(), 74);

    // Search filter
    model.setSearchFilter("benz");
    QCOMPARE(model.rowCount(), 1);
}

QTEST_MAIN(TestSync)
#include "test_sync.moc"
