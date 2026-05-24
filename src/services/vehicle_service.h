#pragma once

#include <QObject>
#include <QString>
#include <QByteArray>

namespace pipesight::comm { class TcpClient; }

namespace pipesight::services {

/**
 * Vehicle (the robot car).
 *
 * Receives telemetry stream (pose / speed / battery / signal) and exposes
 * write APIs for motion + lighting + home.
 *
 * Telemetry is also the source of the odometer value used by LaserService
 * and OsdService — those subscribe to telemetryUpdated().
 */
class VehicleService : public QObject
{
    Q_OBJECT
public:
    struct Telemetry {
        double  pitchDeg   = 0.0;
        double  rollDeg    = 0.0;
        double  yawDeg     = 0.0;
        double  speedMps   = 0.0;
        double  odometerM  = 0.0;
        int     batteryPct = 0;   // 0..100
        double  batteryV   = 0.0;
        int     signalPct  = 0;   // 0..100
    };

    explicit VehicleService(QObject *parent = nullptr);
    ~VehicleService() override;

    Telemetry latest() const { return latest_; }

public slots:
    // Motion: throttle in [-1, 1] (negative = reverse), steer in [-1, 1]
    void drive(qreal throttle, qreal steer);
    void setTargetSpeed(qreal mps);    // for cruise / pre-set gears
    void stop();

    // Lighting: brightness in [0, 100]
    void setFrontLight(int brightness);
    void setRearLight(int brightness);

    // Reset odometer to 0 and (optionally) re-home gimbal.
    void resetHome();

signals:
    void telemetryUpdated(const Telemetry &t);
    void commandAcknowledged(quint8 type);
    void errorOccurred(const QString &msg);

private slots:
    void onFrameReceived(quint8 type, const QByteArray &payload);

private:
    pipesight::comm::TcpClient *client_ = nullptr;
    Telemetry                   latest_;
};

} // namespace pipesight::services
