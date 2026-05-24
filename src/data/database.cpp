#include "database.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QFileInfo>
#include <QDir>
#include <QDebug>

namespace pipesight::data {

Database &Database::instance()
{
    static Database d;
    return d;
}

bool Database::open(const QString &filePath)
{
    QFileInfo fi(filePath);
    QDir().mkpath(fi.absolutePath());

    db_ = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"),
                                    QStringLiteral("pipesight"));
    db_.setDatabaseName(filePath);
    if (!db_.open()) {
        qWarning() << "DB open failed:" << db_.lastError().text();
        return false;
    }
    return migrate();
}

void Database::close()
{
    if (db_.isOpen()) db_.close();
}

bool Database::migrate()
{
    const QStringList ddl = {
        QStringLiteral("CREATE TABLE IF NOT EXISTS sessions ("
                       " id INTEGER PRIMARY KEY AUTOINCREMENT,"
                       " name TEXT, started_at TEXT, ended_at TEXT,"
                       " storage_path TEXT)"),
        QStringLiteral("CREATE TABLE IF NOT EXISTS markers ("
                       " id INTEGER PRIMARY KEY AUTOINCREMENT,"
                       " session_id INTEGER, odometer_m REAL,"
                       " category TEXT, note TEXT,"
                       " created_at TEXT, image_path TEXT)"),
        QStringLiteral("CREATE TABLE IF NOT EXISTS defects ("
                       " id INTEGER PRIMARY KEY AUTOINCREMENT,"
                       " session_id INTEGER, t_sec REAL,"
                       " category TEXT, confidence REAL, image_path TEXT)"),
        QStringLiteral("CREATE TABLE IF NOT EXISTS logs ("
                       " id INTEGER PRIMARY KEY AUTOINCREMENT,"
                       " ts TEXT, level TEXT, source TEXT, message TEXT)")
    };

    QSqlQuery q(db_);
    for (const auto &stmt : ddl) {
        if (!q.exec(stmt)) {
            qWarning() << "DB migrate failed:" << q.lastError().text();
            return false;
        }
    }
    return true;
}

} // namespace pipesight::data
