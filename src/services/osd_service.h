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
    static OsdService &instance();

    ~OsdService() override;

    bool    showProjectInfo() const { return showProject_; }
    bool    showTime()        const { return showTime_; }
    bool    showPosition()    const { return showPosition_; }
    QString projectName()     const { return projectName_; }
    QString inspectionUnit()  const { return inspectionUnit_; }
    QString fontFamily()      const { return fontFamily_; }
    int     fontSize()        const { return fontSize_; }
    int     position()        const { return position_; }
    int     refreshMs()       const { return refreshMs_; }

public slots:
    void setShowProjectInfo(bool on);
    void setShowTime(bool on);
    void setShowPosition(bool on);
    void setProjectName(const QString &name);
    void setInspectionUnit(const QString &unit);
    void setFontFamily(const QString &family);
    void setFontSize(int size);
    void setPosition(int position);
    void setRefreshMs(int ms);

signals:
    void osdChanged();

private:
    explicit OsdService(QObject *parent = nullptr);
    Q_DISABLE_COPY_MOVE(OsdService)

    void loadSettings();

    bool    showProject_   = true;
    bool    showTime_      = true;
    bool    showPosition_  = true;
    QString projectName_;
    QString inspectionUnit_;
    QString fontFamily_;
    int     fontSize_      = 24;
    int     position_      = 0;
    int     refreshMs_     = 1000;
};

} // namespace pipesight::services
