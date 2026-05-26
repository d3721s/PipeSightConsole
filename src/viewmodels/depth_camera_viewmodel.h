#pragma once

#include <QObject>
#include <QString>
#include <QVariant>
#include <QtQmlIntegration/qqmlintegration.h>
#include "services/depth_camera_service.h"

namespace pipesight::viewmodels {

class DepthCameraViewModel : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(bool depthCloudEnabled READ depthCloudEnabled WRITE setDepthCloudEnabled NOTIFY depthCloudEnabledChanged)
    Q_PROPERTY(int  depthMarkerCount  READ depthMarkerCount  NOTIFY depthMarkerCountChanged)

public:
    explicit DepthCameraViewModel(QObject *parent = nullptr);
    ~DepthCameraViewModel() override;

    bool depthCloudEnabled() const { return depthCloud_; }
    int  depthMarkerCount() const { return markerCount_; }
    void setDepthCloudEnabled(bool on);

    Q_INVOKABLE void captureDepthSnapshot()    { service_.captureDepthSnapshot(); }
    Q_INVOKABLE void addDepthMarker(const QString &category, const QString &note)
    {
        service_.addDepthMarker(category, note);
    }
    Q_INVOKABLE void removeDepthMarker(int id) { service_.removeDepthMarker(id); }
    Q_INVOKABLE void setStereoParam(const QString &key, const QVariant &v)
    {
        service_.setStereoParam(key, v);
    }

signals:
    void depthCloudEnabledChanged();
    void depthMarkerCountChanged();

private:
    services::DepthCameraService service_;
    bool                          depthCloud_   = false;
    int                           markerCount_  = 0;
};

} // namespace pipesight::viewmodels
