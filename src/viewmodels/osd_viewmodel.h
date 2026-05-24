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

public:
    explicit OsdViewModel(QObject *parent = nullptr);
    ~OsdViewModel() override;

    bool    showProjectInfo() const { return service_.showProjectInfo(); }
    bool    showTime()        const { return service_.showTime(); }
    bool    showPosition()    const { return service_.showPosition(); }
    QString projectName()     const { return service_.projectName(); }
    QString inspectionUnit()  const { return service_.inspectionUnit(); }

    void setShowProjectInfo(bool on) { service_.setShowProjectInfo(on); }
    void setShowTime(bool on)        { service_.setShowTime(on); }
    void setShowPosition(bool on)    { service_.setShowPosition(on); }
    void setProjectName(const QString &s)    { service_.setProjectName(s); }
    void setInspectionUnit(const QString &s) { service_.setInspectionUnit(s); }

signals:
    void osdChanged();

private:
    services::OsdService service_;
};

} // namespace pipesight::viewmodels
