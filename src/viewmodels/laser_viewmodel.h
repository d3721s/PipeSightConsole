#pragma once

#include <QObject>
#include <QString>
#include <QVariant>
#include <QtQmlIntegration/qqmlintegration.h>
#include "services/laser_service.h"

namespace pipesight::viewmodels {

class LaserViewModel : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(bool pointCloudEnabled READ pointCloudEnabled WRITE setPointCloudEnabled NOTIFY pointCloudEnabledChanged)
    Q_PROPERTY(int  markerCount       READ markerCount       NOTIFY markerCountChanged)

public:
    explicit LaserViewModel(QObject *parent = nullptr);
    ~LaserViewModel() override;

    bool pointCloudEnabled() const { return pointCloud_; }
    int  markerCount() const { return markerCount_; }
    void setPointCloudEnabled(bool on);

    Q_INVOKABLE void captureSnapshot()    { service_.captureSnapshot(); }
    Q_INVOKABLE void addMarker(const QString &category, const QString &note)
    {
        service_.addMarker(category, note);
    }
    Q_INVOKABLE void removeMarker(int id) { service_.removeMarker(id); }
    Q_INVOKABLE void setStereoParam(const QString &key, const QVariant &v)
    {
        service_.setStereoParam(key, v);
    }
    Q_INVOKABLE void setRadarParam(const QString &key, const QVariant &v)
    {
        service_.setRadarParam(key, v);
    }

signals:
    void pointCloudEnabledChanged();
    void markerCountChanged();

private:
    services::LaserService service_;
    bool                    pointCloud_   = false;
    int                     markerCount_  = 0;
};

} // namespace pipesight::viewmodels
