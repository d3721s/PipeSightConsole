#pragma once

#include <QObject>
#include <QString>
#include <QSqlDatabase>

namespace pipesight::data {

/**
 * SQLite-backed persistent store.
 *
 * Schemas (created on open() if missing):
 *   sessions(id, name, started_at, ended_at, storage_path)
 *   markers (id, session_id, odometer_m, category, note, created_at, image_path)
 *   defects (id, session_id, t_sec, category, confidence, image_path)
 *   logs    (id, ts, level, source, message)
 *
 * Lives on whichever thread opens it. Recommended: one Database per
 * thread that writes; or a single one on the main thread with all writes
 * marshalled through a queued slot.
 */
class Database : public QObject
{
    Q_OBJECT
public:
    static Database &instance();

    bool open(const QString &filePath);
    void close();

    QSqlDatabase db() { return db_; }

private:
    Database() = default;
    Q_DISABLE_COPY_MOVE(Database)

    bool migrate();

    QSqlDatabase db_;
};

} // namespace pipesight::data
