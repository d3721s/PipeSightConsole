#pragma once

#include <QObject>
#include <QUrl>
#include <QString>
#include <QByteArray>

namespace pipesight::comm { class TcpClient; }

namespace pipesight::services {

/**
 * Per-camera RTSP credentials + stream selector. Fields map 1:1 to the URL
 * shape `rtsp://{username}:{password}@{ip}:{port}/cam/realmonitor?channel={channel}&subtype={subtype}`.
 */
struct CameraConfig
{
    QString username;
    QString password;
    QString ip;
    quint16 port    = 554;
    int     channel = 1;   // 1-based RTSP channel index
    int     subtype = 0;   // 0 = main stream, 1 = sub stream

    bool isComplete() const { return !ip.isEmpty(); }
};

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

    Channel       activeChannel() const { return active_; }
    QUrl          streamUrl(Channel ch) const;
    CameraConfig  config(Channel ch) const;

public slots:
    // PTZ: dx/dy in [-1, 1] (continuous) or one-shot step.
    void pan(qreal dx, qreal dy);
    void zoom(qreal delta);            // [-1, 1]; positive = zoom in

    void switchChannel(Channel ch);
    void configureCamera(Channel ch, const CameraConfig &cfg);

signals:
    void activeChannelChanged(Channel ch);
    void streamUrlChanged(Channel ch, const QUrl &url);
    void configChanged(Channel ch);
    void ptzAcknowledged();
    void errorOccurred(const QString &msg);

private slots:
    void onFrameReceived(quint8 type, const QByteArray &payload);

private:
    static QUrl    buildUrl(const CameraConfig &cfg);
    static QString settingsPrefix(Channel ch);
    CameraConfig   loadConfig(Channel ch) const;
    void           saveConfig(Channel ch, const CameraConfig &cfg) const;

    pipesight::comm::TcpClient *ptzClient_ = nullptr;
    Channel                     active_ = Channel::Front;
    CameraConfig                frontCfg_;
    CameraConfig                rearCfg_;
};

} // namespace pipesight::services
