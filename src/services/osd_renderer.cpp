#include "osd_renderer.h"
#include "osd_service.h"

#include <QByteArray>
#include <QDir>
#include <QFileInfo>
#include <QIODevice>
#include <QSaveFile>
#include <QStandardPaths>
#include <QStringList>

namespace pipesight::services {

namespace {

QString ffmpegFilterPath(QString path)
{
    path.replace(QLatin1Char('\\'), QLatin1Char('/'));
    path.replace(QLatin1Char(':'), QStringLiteral("\\:"));
    path.replace(QLatin1Char('\''), QStringLiteral("\\'"));
    return path;
}

QString osdFontPath()
{
    const QString yahei = QStringLiteral("C:/Windows/Fonts/msyh.ttc");
    if (QFileInfo::exists(yahei)) return yahei;
    return QStringLiteral("C:/Windows/Fonts/arial.ttf");
}

QDateTime resolvedTimestamp(const QDateTime &timestamp)
{
    return timestamp.isValid() ? timestamp : QDateTime::currentDateTime();
}

} // namespace

OsdRenderer::OsdRenderer()
    : OsdRenderer(OsdService::instance())
{}

OsdRenderer::OsdRenderer(const OsdService &osd)
    : osd_(osd)
{}

QString OsdRenderer::buildText(const OsdTelemetry &telemetry, const QDateTime &timestamp) const
{
    QStringList lines;
    if (osd_.showProjectInfo()) {
        if (!osd_.projectName().isEmpty())
            lines << QStringLiteral("项目：%1").arg(osd_.projectName());
        if (!osd_.inspectionUnit().isEmpty())
            lines << QStringLiteral("单位：%1").arg(osd_.inspectionUnit());
    }
    if (osd_.showTime())
        lines << resolvedTimestamp(timestamp).toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    if (osd_.showPosition()) {
        lines << QStringLiteral("里程：%1 m  速度：%2 m/s")
                     .arg(telemetry.odometerM, 0, 'f', 2)
                     .arg(telemetry.speedMps, 0, 'f', 2);
    }

    return lines.join(QLatin1Char('\n'));
}

bool OsdRenderer::writeTextFile(const QString &filePath,
                                const OsdTelemetry &telemetry,
                                QString *errorMessage,
                                const QDateTime &timestamp) const
{
    if (filePath.isEmpty()) {
        if (errorMessage)
            *errorMessage = QStringLiteral("OSD 文本文件路径为空");
        return false;
    }

    const QFileInfo info(filePath);
    if (!QDir().mkpath(info.absolutePath())) {
        if (errorMessage)
            *errorMessage = QStringLiteral("无法创建 OSD 临时目录：%1").arg(info.absolutePath());
        return false;
    }

    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (errorMessage)
            *errorMessage = QStringLiteral("无法写入 OSD 文本文件：%1").arg(file.errorString());
        return false;
    }

    const QByteArray bytes = buildText(telemetry, timestamp).toUtf8();
    if (file.write(bytes) != bytes.size()) {
        if (errorMessage)
            *errorMessage = QStringLiteral("OSD 文本文件写入不完整：%1").arg(file.errorString());
        return false;
    }

    if (!file.commit()) {
        if (errorMessage)
            *errorMessage = QStringLiteral("无法保存 OSD 文本文件：%1").arg(file.errorString());
        return false;
    }

    return true;
}

QString OsdRenderer::drawTextFilter(const QString &textFilePath) const
{
    if (textFilePath.isEmpty())
        return {};

    return QStringLiteral(
        "drawtext=fontfile='%1':textfile='%2':reload=25:"
        "x=20:y=20:fontsize=24:fontcolor=white:borderw=2:bordercolor=black:"
        "box=1:boxcolor=black@0.35:boxborderw=8")
        .arg(ffmpegFilterPath(osdFontPath()), ffmpegFilterPath(textFilePath));
}

QString OsdRenderer::buildTextFilePath(const QString &prefix)
{
    const QString dir = QDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation))
                            .filePath(QStringLiteral("PipeSightConsole"));
    const QString name = QStringLiteral("%1_%2.txt")
                             .arg(prefix,
                                  QDateTime::currentDateTime().toString(
                                      QStringLiteral("yyyyMMdd_HHmmss_zzz")));
    return QDir(dir).filePath(name);
}

} // namespace pipesight::services
