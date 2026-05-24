#pragma once

#include <QObject>
#include <QImage>
#include <QString>
#include <QByteArray>
#include <QVariant>

namespace pipesight::comm { class TcpClient; }

namespace pipesight::services {

/**
 * Laser / stereo camera service.
 *
 * - Triggers stereo snapshots (used for 3D reconstruction)
 * - Manages marker list (defects / features identified in-frame)
 * - Receives point-cloud / depth frames and emits them for QML rendering
 */
class LaserService : public QObject
{
    Q_OBJECT
public:
    struct Marker {
        int       id;
        double    odometerMeters;  // distance from pipe start (from VehicleService)
        QString   category;        // crack / deformation / joint / ...
        QString   note;
    };

    explicit LaserService(QObject *parent = nullptr);
    ~LaserService() override;

public slots:
    void captureSnapshot();
    void addMarker(const QString &category, const QString &note);
    void removeMarker(int id);
    void setPointCloudEnabled(bool enabled);

    // Stereo camera params (sync / exposure / white balance / ...)
    void setStereoParam(const QString &key, const QVariant &value);
    // Radar params (scan frequency / angle / range)
    void setRadarParam(const QString &key, const QVariant &value);

signals:
    void snapshotReady(const QImage &left, const QImage &right);
    void pointCloudFrame(const QByteArray &xyz);  // raw float3 array
    void markerAdded(const Marker &m);
    void markerRemoved(int id);
    void errorOccurred(const QString &msg);

private slots:
    void onFrameReceived(quint8 type, const QByteArray &payload);

private:
    pipesight::comm::TcpClient *client_ = nullptr;
    int                         nextMarkerId_ = 1;
};

} // namespace pipesight::services
