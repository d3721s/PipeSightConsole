#pragma once

#include <QObject>
#include <QString>

namespace pipesight::services {

/**
 * On-screen-display configuration.
 *
 * Pure UI-side state (no device IO) — what to overlay on the video feed:
 * project info / current time / current position (from VehicleService odometer).
 * Persisted via AppSettings.
 */
class OsdService : public QObject
{
    Q_OBJECT
public:
    explicit OsdService(QObject *parent = nullptr);
    ~OsdService() override;

    bool    showProjectInfo() const { return showProject_; }
    bool    showTime()        const { return showTime_; }
    bool    showPosition()    const { return showPosition_; }
    QString projectName()     const { return projectName_; }
    QString inspectionUnit()  const { return inspectionUnit_; }

public slots:
    void setShowProjectInfo(bool on);
    void setShowTime(bool on);
    void setShowPosition(bool on);
    void setProjectName(const QString &name);
    void setInspectionUnit(const QString &unit);

signals:
    void osdChanged();

private:
    void loadSettings();

    bool    showProject_   = true;
    bool    showTime_      = true;
    bool    showPosition_  = true;
    QString projectName_;
    QString inspectionUnit_;
};

} // namespace pipesight::services
