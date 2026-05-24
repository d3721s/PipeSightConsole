#pragma once

#include <QObject>
#include <QtQmlIntegration/qqmlintegration.h>
#include "services/vehicle_service.h"

namespace pipesight::viewmodels {

class VehicleViewModel : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    // Telemetry (read-only, refreshed by VehicleService::telemetryUpdated)
    Q_PROPERTY(double pitchDeg    READ pitchDeg    NOTIFY telemetryChanged)
    Q_PROPERTY(double rollDeg     READ rollDeg     NOTIFY telemetryChanged)
    Q_PROPERTY(double yawDeg      READ yawDeg      NOTIFY telemetryChanged)
    Q_PROPERTY(double speedMps    READ speedMps    NOTIFY telemetryChanged)
    Q_PROPERTY(double odometerM   READ odometerM   NOTIFY telemetryChanged)
    Q_PROPERTY(int    batteryPct  READ batteryPct  NOTIFY telemetryChanged)
    Q_PROPERTY(double batteryV    READ batteryV    NOTIFY telemetryChanged)
    Q_PROPERTY(int    signalPct   READ signalPct   NOTIFY telemetryChanged)

    // Light control mirrored in VM so QML sliders bind two-way
    Q_PROPERTY(int frontLight READ frontLight WRITE setFrontLight NOTIFY frontLightChanged)
    Q_PROPERTY(int rearLight  READ rearLight  WRITE setRearLight  NOTIFY rearLightChanged)

public:
    explicit VehicleViewModel(QObject *parent = nullptr);
    ~VehicleViewModel() override;

    double pitchDeg()    const { return latest_.pitchDeg; }
    double rollDeg()     const { return latest_.rollDeg; }
    double yawDeg()      const { return latest_.yawDeg; }
    double speedMps()    const { return latest_.speedMps; }
    double odometerM()   const { return latest_.odometerM; }
    int    batteryPct()  const { return latest_.batteryPct; }
    double batteryV()    const { return latest_.batteryV; }
    int    signalPct()   const { return latest_.signalPct; }
    int    frontLight()  const { return frontLight_; }
    int    rearLight()   const { return rearLight_; }

    void setFrontLight(int v);
    void setRearLight(int v);

    Q_INVOKABLE void drive(qreal throttle, qreal steer)
    {
        service_.drive(throttle, steer);
    }
    Q_INVOKABLE void setTargetSpeed(qreal mps) { service_.setTargetSpeed(mps); }
    Q_INVOKABLE void stop()                     { service_.stop(); }
    Q_INVOKABLE void resetHome()                { service_.resetHome(); }

signals:
    void telemetryChanged();
    void frontLightChanged();
    void rearLightChanged();

private:
    services::VehicleService            service_;
    services::VehicleService::Telemetry latest_;
    int                                  frontLight_ = 0;
    int                                  rearLight_  = 0;
};

} // namespace pipesight::viewmodels
