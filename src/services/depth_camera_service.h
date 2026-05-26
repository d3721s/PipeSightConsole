#pragma once

#include <QObject>
#include <QImage>
#include <QString>
#include <QByteArray>
#include <QVariant>

namespace pipesight::comm { class TcpClient; }

namespace pipesight::services {

class DepthCameraService : public QObject
{
    Q_OBJECT
public:
    struct DepthMarker {
        int       id;
        double    odometerMeters;
        QString   category;
        QString   note;
    };

    explicit DepthCameraService(QObject *parent = nullptr);
    ~DepthCameraService() override;

public slots:
    void captureDepthSnapshot();
    void addDepthMarker(const QString &category, const QString &note);
    void removeDepthMarker(int id);
    void setDepthCloudEnabled(bool enabled);

    void setStereoParam(const QString &key, const QVariant &value);

signals:
    void depthSnapshotReady(const QImage &left, const QImage &right);
    void depthCloudFrame(const QByteArray &xyz);
    void depthMarkerAdded(const DepthMarker &m);
    void depthMarkerRemoved(int id);
    void errorOccurred(const QString &msg);

private slots:
    void onFrameReceived(quint8 type, const QByteArray &payload);

private:
    pipesight::comm::TcpClient *client_ = nullptr;
    int                         nextMarkerId_ = 1;
};

} // namespace pipesight::services
