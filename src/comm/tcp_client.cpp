#include "tcp_client.h"
#include "protocol_codec.h"

#include <QTcpSocket>
#include <QTimer>
#include <QDebug>

namespace pipesight::comm {

TcpClient::TcpClient(QString name, QObject *parent)
    : QObject(parent)
    , name_(std::move(name))
    , socket_(new QTcpSocket(this))
    , heartbeatTimer_(new QTimer(this))
    , reconnectTimer_(new QTimer(this))
    , codec_(std::make_unique<ProtocolCodec>())
{
    connect(socket_, &QTcpSocket::connected,    this, &TcpClient::onConnected);
    connect(socket_, &QTcpSocket::disconnected, this, &TcpClient::onDisconnected);
    connect(socket_, &QTcpSocket::readyRead,    this, &TcpClient::onReadyRead);
    connect(socket_, &QTcpSocket::errorOccurred,this, &TcpClient::onSocketError);

    heartbeatTimer_->setSingleShot(false);
    connect(heartbeatTimer_, &QTimer::timeout, this, &TcpClient::onHeartbeatTick);

    reconnectTimer_->setSingleShot(true);
    connect(reconnectTimer_, &QTimer::timeout, this, &TcpClient::onReconnectTick);
}

TcpClient::~TcpClient() = default;

void TcpClient::setHeartbeat(int intervalMs, const QByteArray &payload)
{
    heartbeatIntervalMs_ = intervalMs;
    heartbeatPayload_ = payload;
    if (state_ == State::Connected && intervalMs > 0) {
        heartbeatTimer_->start(intervalMs);
    } else {
        heartbeatTimer_->stop();
    }
}

void TcpClient::setReconnectBackoff(int minMs, int maxMs)
{
    reconnectMinMs_ = minMs;
    reconnectMaxMs_ = maxMs;
    reconnectCurMs_ = minMs;
}

void TcpClient::start(const QString &host, quint16 port)
{
    host_ = host;
    port_ = port;
    userRequestedStop_ = false;
    reconnectCurMs_ = reconnectMinMs_;
    reconnectTimer_->stop();
    heartbeatTimer_->stop();
    rxBuffer_.clear();
    socket_->abort();
    setState(State::Connecting);
    socket_->connectToHost(host, port);
}

void TcpClient::stop()
{
    userRequestedStop_ = true;
    reconnectTimer_->stop();
    heartbeatTimer_->stop();
    socket_->disconnectFromHost();
}

void TcpClient::sendFrame(const QByteArray &payload, quint8 type)
{
    if (state_ != State::Connected) {
        emit errorOccurred(QStringLiteral("send while not connected: %1").arg(name_));
        return;
    }
    const QByteArray bytes = codec_->encode(type, payload);
    socket_->write(bytes);
}

void TcpClient::onConnected()
{
    setState(State::Connected);
    reconnectCurMs_ = reconnectMinMs_;
    if (heartbeatIntervalMs_ > 0)
        heartbeatTimer_->start(heartbeatIntervalMs_);
}

void TcpClient::onDisconnected()
{
    heartbeatTimer_->stop();
    setState(State::Disconnected);
    if (!userRequestedStop_)
        scheduleReconnect();
}

void TcpClient::onReadyRead()
{
    rxBuffer_.append(socket_->readAll());
    auto frames = codec_->feed(rxBuffer_);
    for (const auto &f : frames)
        emit frameReceived(f.type, f.payload);
}

void TcpClient::onSocketError()
{
    emit errorOccurred(socket_->errorString());
}

void TcpClient::onHeartbeatTick()
{
    sendFrame(heartbeatPayload_, /*TYPE_HEARTBEAT*/ 0x01);
}

void TcpClient::onReconnectTick()
{
    if (userRequestedStop_) return;
    setState(State::Connecting);
    socket_->connectToHost(host_, port_);
}

void TcpClient::setState(State s)
{
    if (state_ == s) return;
    state_ = s;
    emit stateChanged(s);
}

void TcpClient::scheduleReconnect()
{
    reconnectTimer_->start(reconnectCurMs_);
    reconnectCurMs_ = qMin(reconnectCurMs_ * 2, reconnectMaxMs_);
}

} // namespace pipesight::comm
