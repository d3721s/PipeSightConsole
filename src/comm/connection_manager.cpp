#include "connection_manager.h"
#include "tcp_client.h"

#include <QThread>

namespace pipesight::comm {

ConnectionManager &ConnectionManager::instance()
{
    static ConnectionManager mgr;
    return mgr;
}

ConnectionManager::ConnectionManager()
    : workerThread_(new QThread)
{
    workerThread_->setObjectName(QStringLiteral("comm-worker"));
    workerThread_->start();
}

ConnectionManager::~ConnectionManager()
{
    for (auto *c : std::as_const(clients_))
        QMetaObject::invokeMethod(c, "stop", Qt::BlockingQueuedConnection);
    qDeleteAll(clients_);
    clients_.clear();

    workerThread_->quit();
    workerThread_->wait();
    delete workerThread_;
}

TcpClient *ConnectionManager::acquire(const QString &name)
{
    if (auto it = clients_.find(name); it != clients_.end())
        return it.value();

    auto *cli = new TcpClient(name);
    cli->moveToThread(workerThread_);
    clients_.insert(name, cli);
    return cli;
}

void ConnectionManager::release(const QString &name)
{
    auto it = clients_.find(name);
    if (it == clients_.end()) return;
    auto *cli = it.value();
    clients_.erase(it);
    QMetaObject::invokeMethod(cli, "stop", Qt::BlockingQueuedConnection);
    cli->deleteLater();
}

} // namespace pipesight::comm
