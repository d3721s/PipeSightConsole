#pragma once

#include <QByteArray>
#include <QList>

namespace pipesight::comm {

/**
 * Length-prefixed binary frame codec.
 *
 *   ┌────────┬────────┬──────┬──────────────┬──────┐
 *   │ MAGIC  │  LEN   │ TYPE │   PAYLOAD    │ CRC  │
 *   │ 0xA55A │ uint16 │ u8   │  LEN-4 bytes │ u16  │
 *   └────────┴────────┴──────┴──────────────┴──────┘
 *
 * LEN = bytes from TYPE through CRC (i.e. 1 + payload.size() + 2).
 * CRC = CRC16-CCITT over [TYPE..PAYLOAD].
 *
 * feed() handles partial frames, multiple frames per read, and resync on
 * desync (drops bytes until next MAGIC).
 *
 * Subclass to swap in a vendor protocol — keep the same Frame/feed/encode
 * shape so higher layers don't change.
 */
class ProtocolCodec
{
public:
    struct Frame {
        quint8     type;
        QByteArray payload;
    };

    ProtocolCodec() = default;
    virtual ~ProtocolCodec() = default;

    virtual QByteArray   encode(quint8 type, const QByteArray &payload) const;
    virtual QList<Frame> feed(QByteArray &buffer);

protected:
    static quint16 crc16Ccitt(const QByteArray &data);
};

} // namespace pipesight::comm
