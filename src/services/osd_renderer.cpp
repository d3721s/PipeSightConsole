#include "osd_renderer.h"
#include "osd_service.h"

#include <QByteArray>
#include <QColor>
#include <QDir>
#include <QFont>
#include <QFontMetrics>
#include <QFileInfo>
#include <QIODevice>
#include <QPainter>
#include <QPoint>
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

QString osdFontPath(QString family)
{
    family = family.trimmed().toLower();

    QStringList candidates;
    if (family == QStringLiteral("simsun")) {
        candidates << QStringLiteral("C:/Windows/Fonts/simsun.ttc");
    } else if (family == QStringLiteral("arial")) {
        candidates << QStringLiteral("C:/Windows/Fonts/arial.ttf");
    } else if (family == QStringLiteral("consolas")) {
        candidates << QStringLiteral("C:/Windows/Fonts/consola.ttf");
    } else {
        candidates << QStringLiteral("C:/Windows/Fonts/msyh.ttc");
    }

    candidates << QStringLiteral("C:/Windows/Fonts/msyh.ttc")
               << QStringLiteral("C:/Windows/Fonts/arial.ttf");

    for (const QString &path : candidates) {
        if (QFileInfo::exists(path))
            return path;
    }
    return candidates.constLast();
}

QPoint osdTextTopLeft(int position, const QSize &imageSize, const QSize &textSize,
                      int margin, int padding)
{
    const int inset = margin + padding;
    int x = inset;
    int y = inset;

    switch (position) {
    case 1:
        x = imageSize.width() - inset - textSize.width();
        break;
    case 2:
        y = imageSize.height() - inset - textSize.height();
        break;
    case 3:
        x = imageSize.width() - inset - textSize.width();
        y = imageSize.height() - inset - textSize.height();
        break;
    default:
        break;
    }

    return QPoint(qMax(inset, x), qMax(inset, y));
}

QString drawTextPosition(int position)
{
    switch (position) {
    case 1:
        return QStringLiteral("x=w-tw-20:y=20");
    case 2:
        return QStringLiteral("x=20:y=h-th-20");
    case 3:
        return QStringLiteral("x=w-tw-20:y=h-th-20");
    default:
        return QStringLiteral("x=20:y=20");
    }
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

QImage OsdRenderer::renderImage(const QImage &source,
                                const OsdTelemetry &telemetry,
                                const QDateTime &timestamp) const
{
    if (source.isNull())
        return {};

    const QString text = buildText(telemetry, timestamp);
    if (text.isEmpty())
        return source;

    QImage image = source.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);

    QFont font(osd_.fontFamily());
    font.setPixelSize(osd_.fontSize());
    painter.setFont(font);

    constexpr int margin = 20;
    constexpr int padding = 8;
    const QRect available(0,
                          0,
                          qMax(0, image.width() - (margin + padding) * 2),
                          qMax(0, image.height() - (margin + padding) * 2));
    const int flags = Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap;
    QRect textRect(QPoint(0, 0), QFontMetrics(font).boundingRect(available, flags, text).size());
    textRect.moveTopLeft(osdTextTopLeft(osd_.position(), image.size(), textRect.size(),
                                        margin, padding));
    const QRect boxRect = textRect.adjusted(-padding, -padding, padding, padding);

    painter.fillRect(boxRect, QColor(0, 0, 0, 90));
    painter.setPen(QColor(0, 0, 0));
    for (int dx = -2; dx <= 2; ++dx) {
        for (int dy = -2; dy <= 2; ++dy) {
            if (dx == 0 && dy == 0) continue;
            painter.drawText(textRect.translated(dx, dy), flags, text);
        }
    }

    painter.setPen(Qt::white);
    painter.drawText(textRect, flags, text);
    return image;
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

    const int reloadFrames = qMax(1, (osd_.refreshMs() + 20) / 40);

    return QStringLiteral(
        "drawtext=fontfile='%1':textfile='%2':reload=%3:"
        "%4:fontsize=%5:fontcolor=white:borderw=2:bordercolor=black:"
        "box=1:boxcolor=black@0.35:boxborderw=8")
        .arg(ffmpegFilterPath(osdFontPath(osd_.fontFamily())),
             ffmpegFilterPath(textFilePath))
        .arg(reloadFrames)
        .arg(drawTextPosition(osd_.position()))
        .arg(osd_.fontSize());
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
