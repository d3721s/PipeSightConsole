#include "app_settings.h"

namespace pipesight::data {

AppSettings &AppSettings::instance()
{
    static AppSettings s;
    return s;
}

AppSettings::AppSettings()
    : settings_(QSettings::IniFormat, QSettings::UserScope,
                QStringLiteral("PipeSight"), QStringLiteral("PipeSightConsole"))
{}

QVariant AppSettings::value(const QString &key, const QVariant &defaultValue) const
{
    return settings_.value(key, defaultValue);
}

void AppSettings::setValue(const QString &key, const QVariant &value)
{
    if (settings_.value(key) == value) return;
    settings_.setValue(key, value);
    emit valueChanged(key);
}

} // namespace pipesight::data
