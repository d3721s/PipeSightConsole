#pragma once

#include <QObject>
#include <QHash>
#include <QString>
#include <memory>

class QThread;

namespace pipesight::comm {

class TcpClient;

/**
 * Owns the Comm worker thread and all named TcpClient instances.
 *
 * Usage from a Service (running on UI thread or comm thread):
 *   auto *cli = ConnectionManager::instance().acquire("camera-front");
 *   connect(cli, &TcpClient::frameReceived, this, &MyService::onFrame);
 *   QMetaObject::invokeMethod(cli, "start", Qt::QueuedConnection,
 *                             Q_ARG(QString, host), Q_ARG(quint16, port));
 *
 * The manager keeps TcpClient ownership; services never delete them.
 */
class ConnectionManager : public QObject
{
    Q_OBJECT
public:
    static ConnectionManager &instance();

    // Returns existing or newly-created client, parented to the worker thread.
    TcpClient *acquire(const QString &name);

    // Stops + destroys the named client (use only on full shutdown).
    void release(const QString &name);

    QThread *workerThread() const { return workerThread_; }

private:
    ConnectionManager();
    ~ConnectionManager() override;
    Q_DISABLE_COPY_MOVE(ConnectionManager)

    QThread                              *workerThread_ = nullptr;
    QHash<QString, TcpClient *>           clients_;
};

} // namespace pipesight::comm
