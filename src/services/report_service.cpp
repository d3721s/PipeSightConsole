#include "report_service.h"
#include <QStandardPaths>
#include <QDesktopServices>
#include <QUrl>
#include <QFile>
#include <QFileInfo>
#include <QDir>

namespace pipesight::services {

ReportService::ReportService(QObject *parent) : QObject(parent) {}
ReportService::~ReportService() = default;

void ReportService::exportReport(ReportFormat fmt, const QString &outFilePath)
{
    Q_UNUSED(fmt);
    emit reportReady(outFilePath);
}

void ReportService::calibrateOdometer()
{
    emit calibrationDone(1.0);
}

QString ReportService::systemLogPath() const
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
           + QStringLiteral("/pipesight.log");
}

bool ReportService::openSystemLog() const
{
    const QString path = systemLogPath();
    if (!QFileInfo::exists(path)) {
        QDir().mkpath(QFileInfo(path).absolutePath());
        QFile f(path);
        if (!f.open(QIODevice::WriteOnly | QIODevice::Append))
            return false;
        f.close();
    }
    return QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

} // namespace pipesight::services
