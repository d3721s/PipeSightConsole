#pragma once

#include <QObject>
#include <QString>

namespace pipesight::services {

class ReportService : public QObject
{
    Q_OBJECT
public:
    enum class ReportFormat { Pdf, Excel };
    Q_ENUM(ReportFormat)

    explicit ReportService(QObject *parent = nullptr);
    ~ReportService() override;

public slots:
    void exportReport(ReportFormat fmt, const QString &outFilePath);

    // Auto-calibrate the odometer against the vehicle's cumulative travel.
    void calibrateOdometer();

    QString systemLogPath() const;

    // Open the system log file in the OS default application.
    bool openSystemLog() const;

signals:
    void reportReady(const QString &filePath);
    void calibrationDone(double scaleFactor);
    void errorOccurred(const QString &msg);
};

} // namespace pipesight::services
