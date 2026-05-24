#pragma once

#include <QObject>
#include <QString>
#include <QtQmlIntegration/qqmlintegration.h>
#include "services/report_service.h"

namespace pipesight::viewmodels {

class AdvancedViewModel : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(int  reportProgress  READ reportProgress  NOTIFY reportProgressChanged)
    Q_PROPERTY(int  firmwareProgress READ firmwareProgress NOTIFY firmwareProgressChanged)
    Q_PROPERTY(QString systemLogPath READ systemLogPath CONSTANT)

public:
    explicit AdvancedViewModel(QObject *parent = nullptr);
    ~AdvancedViewModel() override;

    int     reportProgress()    const { return reportProgress_; }
    int     firmwareProgress()  const { return firmwareProgress_; }
    QString systemLogPath()     const { return service_.systemLogPath(); }

    Q_INVOKABLE void exportPdf(const QString &path)
    {
        service_.exportReport(services::ReportService::ReportFormat::Pdf, path);
    }
    Q_INVOKABLE void exportExcel(const QString &path)
    {
        service_.exportReport(services::ReportService::ReportFormat::Excel, path);
    }
    Q_INVOKABLE void calibrateOdometer(double knownMeters) { service_.calibrateOdometer(knownMeters); }
    Q_INVOKABLE void runDefectRecognition(const QString &path) { service_.runDefectRecognition(path); }
    Q_INVOKABLE void openPlayback(const QString &path)         { service_.openPlayback(path); }
    Q_INVOKABLE void uploadFirmware(const QString &target, const QString &fw)
    {
        service_.uploadFirmware(target, fw);
    }

signals:
    void reportProgressChanged();
    void firmwareProgressChanged();
    void reportReady(QString path);
    void defectFound(QString category, double tSec, double conf);
    void firmwareDone(bool ok, QString msg);

private:
    services::ReportService service_;
    int                      reportProgress_   = 0;
    int                      firmwareProgress_ = 0;
};

} // namespace pipesight::viewmodels
