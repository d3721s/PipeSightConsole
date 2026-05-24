#pragma once

#include <QObject>
#include <QString>
#include <QUrl>
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

    // PTZ controls (called from QML buttons / joystick)
    Q_INVOKABLE void pan(qreal dx, qreal dy)  { service_.pan(dx, dy); }
    Q_INVOKABLE void zoom(qreal delta)         { service_.zoom(delta); }
    Q_INVOKABLE void switchChannel(int channel)
    {
        service_.switchChannel(static_cast<services::CameraService::Channel>(channel));
    }
    Q_INVOKABLE void startRecording();
    Q_INVOKABLE void stopRecording();
    Q_INVOKABLE void snapshot();

    Q_INVOKABLE void configureCamera(int channel, const QString &ip,
                                     const QString &main,
                                     const QString &sub,
                                     const QString &third)
    {
        service_.configureCamera(static_cast<services::CameraService::Channel>(channel),
                                 ip, main, sub, third);
    }

signals:
    void streamUrlChanged();
    void activeChannelChanged();
    void recordingChanged();

private:
    services::CameraService service_;
    bool                     recording_ = false;
};

} // namespace pipesight::viewmodels
