#pragma once

#include <QObject>
#include <QByteArray>
#include <QString>
#include <memory>

class QTcpSocket;
class QTimer;

namespace pipesight::comm {

/**
 * Per-device TCP connection with auto-reconnect and heartbeat.
 *
 * Lives in a worker thread (move to thread before start()). All public
 * slots are queued-safe; all public signals fire from the worker thread —
 * connect to them with Qt::QueuedConnection from the UI thread.
 *
 * Wire framing is delegated to ProtocolCodec: TcpClient just feeds the
 * codec raw bytes and emits frameReceived() for each complete frame.
 */
class TcpClient : public QObject
{
    Q_OBJECT
public:
    enum class State { Disconnected, Connecting, Connected };
    Q_ENUM(State)

    explicit TcpClient(QString name, QObject *parent = nullptr);
    ~TcpClient() override;

    QString name() const { return name_; }
    State   state() const { return state_; }

    // Heartbeat: send a TYPE=0x01 frame every N ms; if no reply in 3x interval,
    // force-reconnect. Set 0 to disable.
    void setHeartbeat(int intervalMs, const QByteArray &payload = {});

    // Reconnect backoff: starts at minMs, doubles up to maxMs.
    void setReconnectBackoff(int minMs, int maxMs);

public slots:
    void start(const QString &host, quint16 port);
    void stop();
    void sendFrame(const QByteArray &payload, quint8 type);

signals:
    void stateChanged(State newState);
    void frameReceived(quint8 type, const QByteArray &payload);
    void errorOccurred(const QString &msg);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onSocketError();
    void onHeartbeatTick();
    void onReconnectTick();

private:
    void setState(State s);
    void scheduleReconnect();

    QString               name_;
    QString               host_;
    quint16               port_ = 0;
    State                 state_ = State::Disconnected;
    QTcpSocket           *socket_ = nullptr;
    QTimer               *heartbeatTimer_ = nullptr;
    QTimer               *reconnectTimer_ = nullptr;
    QByteArray            heartbeatPayload_;
    int                   heartbeatIntervalMs_ = 0;
    int                   reconnectMinMs_ = 1000;
    int                   reconnectMaxMs_ = 30000;
    int                   reconnectCurMs_ = 1000;
    bool                  userRequestedStop_ = false;

    // Frame assembly buffer (handed to ProtocolCodec by feed())
    QByteArray            rxBuffer_;
    std::unique_ptr<class ProtocolCodec> codec_;
};

} // namespace pipesight::comm
