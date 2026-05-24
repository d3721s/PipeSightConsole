#include "report_service.h"
#include <QStandardPaths>

namespace pipesight::services {

ReportService::ReportService(QObject *parent) : QObject(parent) {}
ReportService::~ReportService() = default;

void ReportService::exportReport(ReportFormat fmt, const QString &outFilePath)
{
    Q_UNUSED(fmt); Q_UNUSED(outFilePath);
    // TODO: render via QtPrintSupport (PDF) or QXlsx (Excel) in a worker thread
    emit reportProgress(100);
    emit reportReady(outFilePath);
}

void ReportService::calibrateOdometer(double knownDistanceMeters)
{
    Q_UNUSED(knownDistanceMeters);
    // TODO: compare against VehicleService cumulative odometer, compute scale
    emit calibrationDone(1.0);
}

void ReportService::runDefectRecognition(const QString &videoOrFolderPath)
{
    Q_UNUSED(videoOrFolderPath);
    // TODO: ONNX Runtime in QThreadPool
}

void ReportService::openPlayback(const QString &sessionPath)
{
    Q_UNUSED(sessionPath);
    // TODO: load video + marker list from Database
}

QString ReportService::systemLogPath() const
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
           + QStringLiteral("/pipesight.log");
}

void ReportService::uploadFirmware(const QString &target, const QString &firmwarePath)
{
    Q_UNUSED(target); Q_UNUSED(firmwarePath);
    // TODO: chunked OTA over the device's TcpClient
    emit firmwareDone(false, QStringLiteral("not implemented"));
}

} // namespace pipesight::services
