#include "protocol_codec.h"

#include <QtEndian>

namespace pipesight::comm {

namespace {
constexpr quint16 kMagic = 0xA55A;
constexpr int     kHeaderSize = 4;  // MAGIC(2) + LEN(2)
constexpr int     kTrailerSize = 2; // CRC(2)
}

QByteArray ProtocolCodec::encode(quint8 type, const QByteArray &payload) const
{
    const quint16 innerLen = static_cast<quint16>(1 + payload.size() + kTrailerSize);

    QByteArray out;
    out.reserve(kHeaderSize + innerLen);

    // MAGIC
    char magic[2];
    qToBigEndian<quint16>(kMagic, magic);
    out.append(magic, 2);

    // LEN
    char lenBuf[2];
    qToBigEndian<quint16>(innerLen, lenBuf);
    out.append(lenBuf, 2);

    // TYPE
    out.append(static_cast<char>(type));

    // PAYLOAD
    out.append(payload);

    // CRC over TYPE+PAYLOAD
    const QByteArray crcInput = out.mid(kHeaderSize, 1 + payload.size());
    const quint16 crc = crc16Ccitt(crcInput);
    char crcBuf[2];
    qToBigEndian<quint16>(crc, crcBuf);
    out.append(crcBuf, 2);

    return out;
}

QList<ProtocolCodec::Frame> ProtocolCodec::feed(QByteArray &buffer)
{
    QList<Frame> frames;

    while (true) {
        // Need at least a header to inspect
        if (buffer.size() < kHeaderSize) break;

        // Look for MAGIC
        const quint16 magic = qFromBigEndian<quint16>(buffer.constData());
        if (magic != kMagic) {
            // resync: drop one byte and try again
            buffer.remove(0, 1);
            continue;
        }

        const quint16 innerLen = qFromBigEndian<quint16>(buffer.constData() + 2);
        const int totalLen = kHeaderSize + innerLen;
        if (innerLen < 1 + kTrailerSize) {
            // malformed length; drop magic to resync
            buffer.remove(0, 2);
            continue;
        }
        if (buffer.size() < totalLen) break; // wait for more bytes

        const quint8 type = static_cast<quint8>(buffer[kHeaderSize]);
        const int payloadLen = innerLen - 1 - kTrailerSize;
        const QByteArray payload = buffer.mid(kHeaderSize + 1, payloadLen);
        const quint16 crcGot = qFromBigEndian<quint16>(
            buffer.constData() + kHeaderSize + 1 + payloadLen);

        const QByteArray crcInput = buffer.mid(kHeaderSize, 1 + payloadLen);
        const quint16 crcCalc = crc16Ccitt(crcInput);

        buffer.remove(0, totalLen);

        if (crcGot != crcCalc) {
            // bad CRC: skip frame, keep going
            continue;
        }

        frames.append(Frame{type, payload});
    }

    return frames;
}

quint16 ProtocolCodec::crc16Ccitt(const QByteArray &data)
{
    quint16 crc = 0xFFFF;
    for (char c : data) {
        crc ^= static_cast<quint16>(static_cast<quint8>(c)) << 8;
        for (int i = 0; i < 8; ++i)
            crc = (crc & 0x8000) ? (crc << 1) ^ 0x1021 : (crc << 1);
    }
    return crc;
}

} // namespace pipesight::comm
