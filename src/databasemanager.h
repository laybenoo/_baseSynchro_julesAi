#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QString>
#include <QList>
#include <QSqlDatabase>

struct DrawRecord {
    int tirage = 0;
    int jour = 0;
    int mois = 0;
    int annee = 0;
    int d1 = 0;
    int d2 = 0;
    int d3 = 0;
    int d4 = 0;
    int d5 = 0;
    int total = 0;
};

class DatabaseManager : public QObject {
    Q_OBJECT
public:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    bool openDatabase(const QString &dbPath = "base.db");
    void closeDatabase();

    bool ensureTableExists(const QString &tableName);
    int getMaxLocalDraw(const QString &tableName);
    bool insertDraws(const QString &tableName, const QList<DrawRecord> &records);

private:
    QSqlDatabase m_db;
};

#endif // DATABASEMANAGER_H
