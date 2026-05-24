#pragma once

#include <QObject>
#include <QUrl>
#include <QString>
#include <QByteArray>

namespace pipesight::comm { class TcpClient; }

namespace pipesight::services {

/**
 * Camera (front + rear PTZ IP cameras).
 *
 * - PTZ / zoom control over TCP (vendor protocol via ProtocolCodec)
 * - Video streams over RTSP — exposed as QUrl for QML's MediaPlayer
 * - Recording / snapshot delegated to RecordingService (it owns the writer)
 */
class CameraService : public QObject
{
    Q_OBJECT
public:
    enum class Channel { Front, Rear };
    Q_ENUM(Channel)

    explicit CameraService(QObject *parent = nullptr);
    ~CameraService() override;

    Channel  activeChannel() const { return active_; }
    QUrl     streamUrl(Channel ch) const;

public slots:
    // PTZ: dx/dy in [-1, 1] (continuous) or one-shot step.
    void pan(qreal dx, qreal dy);
    void zoom(qreal delta);            // [-1, 1]; positive = zoom in

    void switchChannel(Channel ch);
    void setStreamUrl(Channel ch, const QUrl &rtsp);

    // Configuration: IP + 3 streams per camera (main / sub / third)
    void configureCamera(Channel ch, const QString &ip,
                         const QString &mainStream,
                         const QString &subStream,
                         const QString &thirdStream);

signals:
    void activeChannelChanged(Channel ch);
    void streamUrlChanged(Channel ch, const QUrl &url);
    void ptzAcknowledged();
    void errorOccurred(const QString &msg);

private slots:
    void onFrameReceived(quint8 type, const QByteArray &payload);

private:
    pipesight::comm::TcpClient *ptzClient_ = nullptr;
    Channel                     active_ = Channel::Front;
    QUrl                        frontUrl_;
    QUrl                        rearUrl_;
};

} // namespace pipesight::services
