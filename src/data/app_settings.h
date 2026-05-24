#pragma once

#include <QObject>
#include <QSettings>
#include <QVariant>

namespace pipesight::data {

/**
 * Thin wrapper around QSettings for typed access + change notifications.
 *
 * Stores: per-camera IP/streams, last storage path, OSD prefs, last-used
 * stereo/radar params. Heavy/structured data goes to Database, not here.
 */
class AppSettings : public QObject
{
    Q_OBJECT
public:
    static AppSettings &instance();

    QVariant value(const QString &key, const QVariant &defaultValue = {}) const;
    void     setValue(const QString &key, const QVariant &value);

signals:
    void valueChanged(const QString &key);

private:
    AppSettings();
    Q_DISABLE_COPY_MOVE(AppSettings)

    mutable QSettings settings_;
};

} // namespace pipesight::data
