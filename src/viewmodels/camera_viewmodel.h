#pragma once

#include <QObject>
#include <QString>
#include <QUrl>
#include <QVariantMap>
#include <QtQmlIntegration/qqmlintegration.h>
#include "services/camera_service.h"

namespace pipesight::viewmodels {

class CameraViewModel : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QUrl streamUrl       READ streamUrl        NOTIFY streamUrlChanged)
    Q_PROPERTY(int  activeChannel   READ activeChannelInt NOTIFY activeChannelChanged)
    Q_PROPERTY(bool recording       READ recording        NOTIFY recordingChanged)

public:
    explicit CameraViewModel(QObject *parent = nullptr);
    ~CameraViewModel() override;

    QUrl streamUrl() const;
    int  activeChannelInt() const { return static_cast<int>(service_.activeChannel()); }
    bool recording() const { return recording_; }

    Q_INVOKABLE void pan(qreal dx, qreal dy)  { service_.pan(dx, dy); }
    Q_INVOKABLE void zoom(qreal delta)         { service_.zoom(delta); }
    Q_INVOKABLE void switchChannel(int channel)
    {
        service_.switchChannel(static_cast<services::CameraService::Channel>(channel));
    }
    Q_INVOKABLE void startRecording();
    Q_INVOKABLE void stopRecording();
    Q_INVOKABLE void snapshot();

    Q_INVOKABLE void configureCamera(int channel,
                                     const QString &username,
                                     const QString &password,
                                     const QString &ip,
                                     int port,
                                     int rtspChannel,
                                     int subtype);

    Q_INVOKABLE QVariantMap cameraConfig(int channel) const;
    Q_INVOKABLE QString     cameraStreamUrl(int channel) const;

signals:
    void streamUrlChanged();
    void activeChannelChanged();
    void recordingChanged();
    void cameraConfigChanged(int channel);

private:
    services::CameraService service_;
    bool                     recording_ = false;
};

} // namespace pipesight::viewmodels
