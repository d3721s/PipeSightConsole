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

public:
    explicit AdvancedViewModel(QObject *parent = nullptr);
    ~AdvancedViewModel() override;

    Q_INVOKABLE void exportPdf(const QString &path)
    {
        service_.exportReport(services::ReportService::ReportFormat::Pdf, path);
    }
    Q_INVOKABLE void exportExcel(const QString &path)
    {
        service_.exportReport(services::ReportService::ReportFormat::Excel, path);
    }
    Q_INVOKABLE void calibrateOdometer() { service_.calibrateOdometer(); }
    Q_INVOKABLE bool openSystemLog()     { return service_.openSystemLog(); }

signals:
    void reportReady(QString path);
    void calibrationDone(double scaleFactor);

private:
    services::ReportService service_;
};

} // namespace pipesight::viewmodels
