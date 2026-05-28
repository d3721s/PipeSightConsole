#pragma once

#include <QObject>
#include <QString>
#include <QVariant>
#include <QtQmlIntegration/qqmlintegration.h>
#include "services/recording_service.h"
#include "services/vehicle_service.h"

namespace pipesight::viewmodels {

/**
 * ConfigViewModel covers the "配置" panel: camera IPs + 3 streams each,
 * vehicle IP, stereo camera params, radar params, recording config.
 *
 * It owns RecordingService directly (it's the closest match) and forwards
 * camera/laser-radar config through the respective Services via signals
 * the App composes in main.cpp.
 */
class ConfigViewModel : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(int     segmentMinutes READ segmentMinutes WRITE setSegmentMinutes NOTIFY configChanged)
    Q_PROPERTY(QString storagePath    READ storagePath    WRITE setStoragePath    NOTIFY configChanged)
    Q_PROPERTY(int     recordingCodec READ recordingCodec WRITE setRecordingCodec NOTIFY configChanged)
    Q_PROPERTY(int     recordingMode  READ recordingMode  WRITE setRecordingMode  NOTIFY configChanged)
    Q_PROPERTY(QString vehicleIp      READ vehicleIp      WRITE setVehicleIp      NOTIFY configChanged)
    Q_PROPERTY(int     vehicleInfoRefreshMs READ vehicleInfoRefreshMs WRITE setVehicleInfoRefreshMs NOTIFY configChanged)

public:
    explicit ConfigViewModel(QObject *parent = nullptr);
    ~ConfigViewModel() override;

    int     segmentMinutes() const { return recording_.segmentMinutes(); }
    QString storagePath()    const { return recording_.storagePath(); }
    int     recordingCodec() const { return static_cast<int>(recording_.codec()); }
    int     recordingMode()  const { return static_cast<int>(recording_.encodingMode()); }
    QString vehicleIp()      const { return vehicle_.ip(); }
    int     vehicleInfoRefreshMs() const { return vehicle_.infoRefreshMs(); }

    void setSegmentMinutes(int m);
    void setStoragePath(const QString &p);
    void setRecordingCodec(int codec);
    void setRecordingMode(int mode);
    void setVehicleIp(const QString &ip);
    void setVehicleInfoRefreshMs(int ms);

    Q_INVOKABLE bool applyRecordingConfig(int segmentMinutes,
                                          const QString &storagePath,
                                          int recordingCodec,
                                          int recordingMode);
    Q_INVOKABLE void configureCamera(int channel, const QString &ip,
                                     const QString &main,
                                     const QString &sub,
                                     const QString &third);
    Q_INVOKABLE QVariant stereoParam(const QString &key) const;
    Q_INVOKABLE QVariant radarParam(const QString &key) const;
    Q_INVOKABLE void setStereoParam(const QString &key, const QVariant &v);
    Q_INVOKABLE void setRadarParam(const QString &key, const QVariant &v);
    Q_INVOKABLE bool readStereoParams();
    Q_INVOKABLE bool applyStereoParams(int exposure, const QString &whiteBalance, bool sync);
    Q_INVOKABLE bool applyVehicleConfig(const QString &ip, int infoRefreshMs);
    Q_INVOKABLE bool readRadarParams();
    Q_INVOKABLE bool applyRadarParams(int scanHz, int angleDeg, int rangeM);

signals:
    void configChanged();
    void cameraConfigured(int channel, QString ip, QString mainStream, QString subStream, QString thirdStream);
    void stereoParamChanged(QString key, QVariant value);
    void radarParamChanged(QString key, QVariant value);

private:
    services::RecordingService recording_;
    services::VehicleService &vehicle_;
};

} // namespace pipesight::viewmodels
