#pragma once

#include <QDateTime>
#include <QImage>
#include <QString>

namespace pipesight::services {

class OsdService;

struct OsdTelemetry
{
    double odometerM = 0.0;
    double speedMps = 0.0;
};

/**
 * Converts the current OSD configuration and live telemetry into concrete
 * render artifacts used by file writers.
 */
class OsdRenderer
{
public:
    OsdRenderer();
    explicit OsdRenderer(const OsdService &osd);

    QString buildText(const OsdTelemetry &telemetry, const QDateTime &timestamp = {}) const;
    QImage renderImage(const QImage &source,
                       const OsdTelemetry &telemetry,
                       const QDateTime &timestamp = {}) const;
    bool writeTextFile(const QString &filePath,
                       const OsdTelemetry &telemetry,
                       QString *errorMessage = nullptr,
                       const QDateTime &timestamp = {}) const;
    QString drawTextFilter(const QString &textFilePath) const;

    static QString buildTextFilePath(const QString &prefix);

private:
    const OsdService &osd_;
};

} // namespace pipesight::services
