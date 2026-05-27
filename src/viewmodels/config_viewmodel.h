#pragma once

#include <QObject>
#include <QString>
#include <QVariant>
#include <QtQmlIntegration/qqmlintegration.h>
#include "services/recording_service.h"

namespace pipesight::viewmodels {

/**
 * ConfigViewModel covers the "配置" panel: camera IPs + 3 streams each,
 * stereo camera params, radar params, recording config.
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
    Q_PROPERTY(bool    cyclicRecord   READ cyclicRecord   WRITE setCyclicRecord   NOTIFY configChanged)
    Q_PROPERTY(QString storagePath    READ storagePath    WRITE setStoragePath    NOTIFY configChanged)

public:
    explicit ConfigViewModel(QObject *parent = nullptr);
    ~ConfigViewModel() override;

    int     segmentMinutes() const { return recording_.segmentMinutes(); }
    bool    cyclicRecord()   const { return recording_.cyclicEnabled(); }
    QString storagePath()    const { return recording_.storagePath(); }

    void setSegmentMinutes(int m);
    void setCyclicRecord(bool on);
    void setStoragePath(const QString &p);

    Q_INVOKABLE void configureCamera(int channel, const QString &ip,
                                     const QString &main,
                                     const QString &sub,
                                     const QString &third);
    Q_INVOKABLE QVariant stereoParam(const QString &key) const;
    Q_INVOKABLE QVariant radarParam(const QString &key) const;
    Q_INVOKABLE void setStereoParam(const QString &key, const QVariant &v);
    Q_INVOKABLE void setRadarParam(const QString &key, const QVariant &v);

signals:
    void configChanged();
    void cameraConfigured(int channel, QString ip, QString mainStream, QString subStream, QString thirdStream);
    void stereoParamChanged(QString key, QVariant value);
    void radarParamChanged(QString key, QVariant value);

private:
    services::RecordingService recording_;
};

} // namespace pipesight::viewmodels
