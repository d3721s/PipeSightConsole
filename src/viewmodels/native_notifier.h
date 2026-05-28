#pragma once

#include <QObject>
#include <QString>
#include <QtQmlIntegration/qqmlintegration.h>

namespace pipesight::viewmodels {

class NativeNotifier : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(bool available READ available CONSTANT)

public:
    explicit NativeNotifier(QObject *parent = nullptr);
    ~NativeNotifier() override;

    bool available() const;

    Q_INVOKABLE bool requestPermission();
    Q_INVOKABLE bool notificationsEnabled() const;
    Q_INVOKABLE bool notify(const QString &title, const QString &message);
    Q_INVOKABLE bool notifyError(const QString &title, const QString &message);

signals:
    void notificationFailed(QString message);

private:
    bool sendNotification(const QString &title, const QString &message, bool isError);
};

} // namespace pipesight::viewmodels
