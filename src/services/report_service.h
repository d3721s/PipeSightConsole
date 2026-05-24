#pragma once

#include <QObject>
#include <QString>

namespace pipesight::services {

/**
 * Advanced features umbrella: report export, calibration, AI defect
 * detection, playback, log access, firmware upgrade.
 *
 * Most of these are local-only (no TCP) except FW upgrade which streams
 * a file to the device. AI detection runs ONNX Runtime in a worker pool.
 */
class ReportService : public QObject
{
    Q_OBJECT
public:
    enum class ReportFormat { Pdf, Excel };
    Q_ENUM(ReportFormat)

    explicit ReportService(QObject *parent = nullptr);
    ~ReportService() override;

public slots:
    // Generate a report for the active project; emits reportReady on success.
    void exportReport(ReportFormat fmt, const QString &outFilePath);

    // Calibrate the odometer against a known traversed distance.
    void calibrateOdometer(double knownDistanceMeters);

    // Trigger AI defect recognition on the recorded segment(s).
    void runDefectRecognition(const QString &videoOrFolderPath);

    // Open/load a recorded session for offline playback + marker editing.
    void openPlayback(const QString &sessionPath);

    // Log file access.
    QString systemLogPath() const;

    // Firmware OTA upload to the car/cameras.
    void uploadFirmware(const QString &target, const QString &firmwarePath);

signals:
    void reportProgress(int percent);
    void reportReady(const QString &filePath);
    void calibrationDone(double scaleFactor);
    void defectFound(const QString &category, double timestampSec, double confidence);
    void firmwareProgress(int percent);
    void firmwareDone(bool ok, const QString &msg);
    void errorOccurred(const QString &msg);
};

} // namespace pipesight::services
