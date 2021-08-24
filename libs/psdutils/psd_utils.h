/*
 *  SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef PSD_UTILS_H
#define PSD_UTILS_H

#include "kritapsdutils_export.h"

#include <QtEndian>
#include <QtGlobal>
#include <array>
#include <psd.h>
#include <resources/KoPattern.h>
#include <type_traits>

class QIODevice;
class QString;

/**
 * Writing functions.
 */

inline bool psdwriteBE(QIODevice &io, const quint8 &v)
{
    const std::array<quint8, 2> val = {v};
    const qint64 written = io.write(reinterpret_cast<const char *>(val.data()), 1);
    return written == 1;
}

inline bool psdwriteLE(QIODevice &io, const quint8 &v)
{
    const std::array<quint8, 2> val = {v};
    const qint64 written = io.write(reinterpret_cast<const char *>(val.data()), 1);
    return written == 1;
}

inline bool psdwriteBE(QIODevice &io, const quint16 &v)
{
    const std::array<quint8, 2> val = {
        quint8(v >> 8U),
        quint8(v),
    };
    const qint64 written = io.write(reinterpret_cast<const char *>(val.data()), 2);
    return written == 2;
}

inline bool psdwriteLE(QIODevice &io, const quint16 &v)
{
    const std::array<quint8, 2> val = {
        quint8(v),
        quint8(v >> 8U),
    };
    const qint64 written = io.write(reinterpret_cast<const char *>(val.data()), 2);
    return written == 2;
}

inline bool psdwriteBE(QIODevice &io, const quint32 &v)
{
    const std::array<quint8, 4> val = {
        quint8(v >> 24U),
        quint8(v >> 16U),
        quint8(v >> 8U),
        quint8(v),
    };
    const qint64 written = io.write(reinterpret_cast<const char *>(val.data()), 4);
    return written == 4;
}

inline bool psdwriteLE(QIODevice &io, const quint32 &v)
{
    const std::array<quint8, 4> val = {
        quint8(v),
        quint8(v >> 8U),
        quint8(v >> 16U),
        quint8(v >> 24U),
    };
    const qint64 written = io.write(reinterpret_cast<const char *>(val.data()), 4);
    return written == 4;
}

inline bool psdwriteBE(QIODevice &io, const quint64 &v)
{
    const std::array<quint8, 8> val = {
        quint8(v >> 56U),
        quint8(v >> 48U),
        quint8(v >> 40U),
        quint8(v >> 32U),
        quint8(v >> 24U),
        quint8(v >> 16U),
        quint8(v >> 8U),
        quint8(v),
    };
    const qint64 written = io.write(reinterpret_cast<const char *>(val.data()), 8);
    return written == 8;
}

inline bool psdwriteLE(QIODevice &io, const quint64 &v)
{
    const std::array<quint8, 8> val = {
        quint8(v),
        quint8(v >> 8U),
        quint8(v >> 16U),
        quint8(v >> 24U),
        quint8(v >> 32U),
        quint8(v >> 40U),
        quint8(v >> 48U),
        quint8(v >> 56U),
    };
    const qint64 written = io.write(reinterpret_cast<const char *>(val.data()), 8);
    return written == 8;
}

/**
 * Templated writing fallbacks for non-integral types.
 */

template<typename T>
inline bool psdwriteBE(QIODevice &io, std::enable_if_t<sizeof(T) == sizeof(quint8), T &> v)
{
    return psdwriteBE(io, reinterpret_cast<quint8 &>(v));
}

template<typename T>
inline bool psdwriteBE(QIODevice &io, std::enable_if_t<sizeof(T) == sizeof(quint16), T &> v)
{
    return psdwriteBE(io, reinterpret_cast<quint16 &>(v));
}

template<typename T>
inline bool psdwriteBE(QIODevice &io, std::enable_if_t<sizeof(T) == sizeof(quint32), T &> v)
{
    return psdwriteBE(io, reinterpret_cast<quint32 &>(v));
}

template<typename T>
inline bool psdwriteBE(QIODevice &io, std::enable_if_t<sizeof(T) == sizeof(quint64), T &> v)
{
    return psdwriteBE(io, reinterpret_cast<quint64 &>(v));
}

template<typename T>
inline bool psdwriteLE(QIODevice &io, std::enable_if_t<sizeof(T) == sizeof(quint8), T &> v)
{
    return psdwriteLE(io, reinterpret_cast<quint8 &>(v));
}

template<typename T>
inline bool psdwriteLE(QIODevice &io, std::enable_if_t<sizeof(T) == sizeof(quint16), T &> v)
{
    return psdwriteLE(io, reinterpret_cast<quint16 &>(v));
}

template<typename T>
inline bool psdwriteLE(QIODevice &io, std::enable_if_t<sizeof(T) == sizeof(quint32), T &> v)
{
    return psdwriteLE(io, reinterpret_cast<quint32 &>(v));
}

template<typename T>
inline bool psdwriteLE(QIODevice &io, std::enable_if_t<sizeof(T) == sizeof(quint64), T &> v)
{
    return psdwriteLE(io, reinterpret_cast<quint64 &>(v));
}

template<psd_byte_order byteOrder = psd_byte_order::psdBigEndian, typename T>
inline std::enable_if_t<std::is_arithmetic<T>::value, bool> psdwrite(QIODevice &io, T v)
{
    if (byteOrder == psd_byte_order::psdLittleEndian) {
        return psdwriteLE<T>(io, v);
    } else {
        return psdwriteBE<T>(io, v);
    }
}

inline bool psdwrite(QIODevice &io, const QString &s)
{
    const QByteArray b = s.toLatin1();
    int l = b.size();
    const qint64 written = io.write(reinterpret_cast<const char *>(b.data()), l);
    return written == l;
}

template<psd_byte_order byteOrder = psd_byte_order::psdBigEndian>
inline bool psdwrite_pascalstring(QIODevice &io, const QString &s)
{
    Q_ASSERT(s.length() < 256);
    Q_ASSERT(s.length() >= 0);
    if (s.length() < 0 || s.length() > 255)
        return false;

    if (s.isNull()) {
        psdwrite<byteOrder>(io, quint8(0));
        psdwrite<byteOrder>(io, quint8(0));
        return true;
    }

    quint8 length = static_cast<quint8>(s.length());
    psdwrite<byteOrder>(io, length);

    const QByteArray b = s.toLatin1();
    const qint64 written = io.write(reinterpret_cast<const char *>(b.data()), length);
    if (written != length)
        return false;

    if ((length & 0x01) != 0) {
        return psdwrite<byteOrder>(io, quint8(0));
    }

    return true;
}

template<psd_byte_order byteOrder = psd_byte_order::psdBigEndian>
inline bool psdwrite_pascalstring(QIODevice &io, const QString &s, int padding)
{
    Q_ASSERT(s.length() < 256);
    Q_ASSERT(s.length() >= 0);
    if (s.length() < 0 || s.length() > 255)
        return false;

    if (s.isNull()) {
        psdwrite<byteOrder>(io, quint8(0));
        psdwrite<byteOrder>(io, quint8(0));
        return true;
    }
    quint8 length = static_cast<quint8>(s.length());
    psdwrite<byteOrder>(io, length);

    QByteArray b = s.toLatin1();
    const qint64 written = io.write(b.data(), length);
    if (written != length)
        return false;

    // If the total length (length byte + content) is not a multiple of padding, add zeroes to pad
    length++;
    if ((length % padding) != 0) {
        for (int i = 0; i < (padding - (length % padding)); i++) {
            psdwrite<byteOrder>(io, quint8(0));
        }
    }

    return true;
}

inline bool psdpad(QIODevice &io, quint32 padding)
{
    for (quint32 i = 0; i < padding; i++) {
        const bool written = io.putChar('\0');
        if (!written)
            return false;
    }
    return true;
}

/**
 * Reading functions.
 */

inline bool psdreadBE(QIODevice &io, quint8 &v)
{
    std::array<quint8, 1> data;
    qint64 read = io.read(reinterpret_cast<char *>(data.data()), 1);
    if (read != 1)
        return false;
    v = data[0];
    return true;
}

inline bool psdreadLE(QIODevice &io, quint8 &v)
{
    std::array<quint8, 1> data;
    qint64 read = io.read(reinterpret_cast<char *>(data.data()), 1);
    if (read != 1)
        return false;
    v = data[0];
    return true;
}

inline bool psdreadBE(QIODevice &io, quint16 &v)
{
    std::array<quint8, 2> data;
    qint64 read = io.read(reinterpret_cast<char *>(data.data()), 2);
    if (read != 2)
        return false;
    v = quint16((quint16(data[0]) << 8U) | data[1]);
    return true;
}

inline bool psdreadLE(QIODevice &io, quint16 &v)
{
    std::array<quint8, 2> data;
    qint64 read = io.read(reinterpret_cast<char *>(data.data()), 2);
    if (read != 2)
        return false;
    v = quint16((quint16(data[1]) << 8U) | data[0]);
    return true;
}

inline bool psdreadBE(QIODevice &io, quint32 &v)
{
    std::array<quint8, 4> data;
    qint64 read = io.read(reinterpret_cast<char *>(data.data()), 4);
    if (read != 4)
        return false;
    v = (quint32(data[0]) << 24U) | (quint32(data[1]) << 16U) | (quint32(data[2]) << 8U) | data[3];
    return true;
}

inline bool psdreadLE(QIODevice &io, quint32 &v)
{
    std::array<quint8, 4> data;
    qint64 read = io.read(reinterpret_cast<char *>(data.data()), 4);
    if (read != 4)
        return false;
    v = (quint32(data[3]) << 24U) | (quint32(data[2]) << 16U) | (quint32(data[1]) << 8U) | data[0];
    return true;
}

inline bool psdreadBE(QIODevice &io, quint64 &v)
{
    std::array<quint8, 8> data;
    qint64 read = io.read(reinterpret_cast<char *>(data.data()), 8);
    if (read != 8)
        return false;
    v = (quint64(data[0]) << 56U) | (quint64(data[1]) << 48U) | (quint64(data[2]) << 40U) | (quint64(data[3]) << 32U) | (quint64(data[4]) << 24U)
        | (quint64(data[5]) << 16U) | (quint64(data[6]) << 8U) | data[7];
    return true;
}

inline bool psdreadLE(QIODevice &io, quint64 &v)
{
    std::array<quint8, 8> data;
    qint64 read = io.read(reinterpret_cast<char *>(data.data()), 8);
    if (read != 8)
        return false;
    v = (quint64(data[7]) << 56U) | (quint64(data[6]) << 48U) | (quint64(data[5]) << 40U) | (quint64(data[4]) << 32U) | (quint64(data[3]) << 24U)
        | (quint64(data[2]) << 16U) | (quint64(data[1]) << 8U) | data[0];
    return true;
}

/**
 * Templated reading fallbacks for non-integral types.
 */

template<typename T>
inline bool psdreadBE(QIODevice &io, std::enable_if_t<sizeof(T) == sizeof(quint8), T &> v)
{
    return psdreadBE(io, reinterpret_cast<quint8 &>(v));
}

template<typename T>
inline bool psdreadBE(QIODevice &io, std::enable_if_t<sizeof(T) == sizeof(quint16), T &> v)
{
    return psdreadBE(io, reinterpret_cast<quint16 &>(v));
}

template<typename T>
inline bool psdreadBE(QIODevice &io, std::enable_if_t<sizeof(T) == sizeof(quint32), T &> v)
{
    return psdreadBE(io, reinterpret_cast<quint32 &>(v));
}

template<typename T>
inline bool psdreadBE(QIODevice &io, std::enable_if_t<sizeof(T) == sizeof(quint64), T &> v)
{
    return psdreadBE(io, reinterpret_cast<quint64 &>(v));
}

template<typename T>
inline bool psdreadLE(QIODevice &io, std::enable_if_t<sizeof(T) == sizeof(quint8), T &> v)
{
    return psdreadLE(io, reinterpret_cast<quint8 &>(v));
}

template<typename T>
inline bool psdreadLE(QIODevice &io, std::enable_if_t<sizeof(T) == sizeof(quint16), T &> v)
{
    return psdreadLE(io, reinterpret_cast<quint16 &>(v));
}

template<typename T>
inline bool psdreadLE(QIODevice &io, std::enable_if_t<sizeof(T) == sizeof(quint32), T &> v)
{
    return psdreadLE(io, reinterpret_cast<quint32 &>(v));
}

template<typename T>
inline bool psdreadLE(QIODevice &io, std::enable_if_t<sizeof(T) == sizeof(quint64), T &> v)
{
    return psdreadLE(io, reinterpret_cast<quint64 &>(v));
}

template<psd_byte_order byteOrder = psd_byte_order::psdBigEndian, typename T>
inline std::enable_if_t<std::is_arithmetic<T>::value, bool> psdread(QIODevice &io, T &v)
{
    if (byteOrder == psd_byte_order::psdLittleEndian) {
        return psdreadLE<T>(io, v);
    } else {
        return psdreadBE<T>(io, v);
    }
}

template<psd_byte_order byteOrder = psd_byte_order::psdBigEndian>
inline QByteArray psdreadBytes(QIODevice &io, qint64 v)
{
    QByteArray b = io.read(v);
    if (byteOrder == psd_byte_order::psdLittleEndian) {
        std::reverse(b.begin(), b.end());
    }
    return b;
}

template<psd_byte_order byteOrder = psd_byte_order::psdBigEndian>
inline bool psdread_pascalstring(QIODevice &io, QString &s, qint64 padding)
{
    quint8 length;
    if (!psdread<byteOrder>(io, length)) {
        return false;
    }

    if (length == 0) {
        // read the padding
        for (qint64 i = 0; i < padding - 1; ++i) {
            io.seek(io.pos() + 1);
        }
        return true;
    }

    QByteArray chars = io.read(length);
    if (chars.length() != length) {
        return false;
    }

    // read padding byte
    quint32 paddedLength = length + 1;
    if (padding > 0) {
        while (paddedLength % padding != 0) {
            if (!io.seek(io.pos() + 1)) {
                return false;
            }
            paddedLength++;
        }
    }

    s.append(QString::fromLatin1(chars));

    return true;
}

template<psd_byte_order byteOrder = psd_byte_order::psdBigEndian>
inline bool psdread_unicodestring(QIODevice &io, QString &s)
{
    quint32 stringlen;
    if (!psdread<byteOrder>(io, stringlen)) {
        return false;
    }

    s.reserve(static_cast<int>(stringlen));

    for (quint32 i = 0; i < stringlen; ++i) {
        quint16 ch(0);
        if (!psdread<byteOrder>(io, ch)) {
            return false;
        }

        // XXX: this makes it possible to append garbage
        if (ch != 0) {
            s[i] = QChar(ch);
        }
    }

    return true;
}

template<psd_byte_order byteOrder = psd_byte_order::psdBigEndian>
inline bool psd_read_blendmode(QIODevice &io, QString &blendModeKey)
{
    QByteArray b(psdreadBytes<byteOrder>(io, 4));
    if (b.size() != 4 || QString(b) != "8BIM") {
        return false;
    }
    blendModeKey = QString(psdreadBytes<byteOrder>(io, 4));
    if (blendModeKey.size() != 4) {
        return false;
    }
    return true;
}

#endif // PSD_UTILS_H
