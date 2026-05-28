#pragma once

#include <QObject>
#include <QString>
#include <QtQmlIntegration/qqmlintegration.h>
#include "services/osd_service.h"

namespace pipesight::viewmodels {

class OsdViewModel : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(bool    showProjectInfo READ showProjectInfo WRITE setShowProjectInfo NOTIFY osdChanged)
    Q_PROPERTY(bool    showTime        READ showTime        WRITE setShowTime        NOTIFY osdChanged)
    Q_PROPERTY(bool    showPosition    READ showPosition    WRITE setShowPosition    NOTIFY osdChanged)
    Q_PROPERTY(QString projectName     READ projectName     WRITE setProjectName     NOTIFY osdChanged)
    Q_PROPERTY(QString inspectionUnit  READ inspectionUnit  WRITE setInspectionUnit  NOTIFY osdChanged)
    Q_PROPERTY(QString fontFamily      READ fontFamily      WRITE setFontFamily      NOTIFY osdChanged)
    Q_PROPERTY(int     fontSize        READ fontSize        WRITE setFontSize        NOTIFY osdChanged)
    Q_PROPERTY(int     position        READ position        WRITE setPosition        NOTIFY osdChanged)
    Q_PROPERTY(int     refreshMs       READ refreshMs       WRITE setRefreshMs       NOTIFY osdChanged)

public:
    explicit OsdViewModel(QObject *parent = nullptr);
    ~OsdViewModel() override;

    bool    showProjectInfo() const { return service_.showProjectInfo(); }
    bool    showTime()        const { return service_.showTime(); }
    bool    showPosition()    const { return service_.showPosition(); }
    QString projectName()     const { return service_.projectName(); }
    QString inspectionUnit()  const { return service_.inspectionUnit(); }
    QString fontFamily()      const { return service_.fontFamily(); }
    int     fontSize()        const { return service_.fontSize(); }
    int     position()        const { return service_.position(); }
    int     refreshMs()       const { return service_.refreshMs(); }

    void setShowProjectInfo(bool on) { service_.setShowProjectInfo(on); }
    void setShowTime(bool on)        { service_.setShowTime(on); }
    void setShowPosition(bool on)    { service_.setShowPosition(on); }
    void setProjectName(const QString &s)    { service_.setProjectName(s); }
    void setInspectionUnit(const QString &s) { service_.setInspectionUnit(s); }
    void setFontFamily(const QString &s)     { service_.setFontFamily(s); }
    void setFontSize(int size)               { service_.setFontSize(size); }
    void setPosition(int position)           { service_.setPosition(position); }
    void setRefreshMs(int ms)                { service_.setRefreshMs(ms); }

signals:
    void osdChanged();

private:
    services::OsdService &service_;
};

} // namespace pipesight::viewmodels
