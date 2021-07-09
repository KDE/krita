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
#include <algorithm>
#include <array>
#include <psd.h>
#include <resources/KoPattern.h>

class QIODevice;
class QString;

bool KRITAPSDUTILS_EXPORT psdwrite(QIODevice *io, quint8 v);
bool KRITAPSDUTILS_EXPORT psdwrite(QIODevice *io, quint16 v);
bool KRITAPSDUTILS_EXPORT psdwrite(QIODevice *io, qint16 v);
bool KRITAPSDUTILS_EXPORT psdwrite(QIODevice *io, quint32 v);
bool KRITAPSDUTILS_EXPORT psdwrite(QIODevice *io, const QString &s);
bool KRITAPSDUTILS_EXPORT psdwrite(QIODevice *io, double v);
bool KRITAPSDUTILS_EXPORT psdwrite_pascalstring(QIODevice *io, const QString &s);
bool KRITAPSDUTILS_EXPORT psdwrite_pascalstring(QIODevice *io, const QString &s, int padding);
bool KRITAPSDUTILS_EXPORT psdpad(QIODevice *io, quint32 padding);

/**
 * Reading functions.
 */

inline bool psdreadBE(QIODevice *io, quint8 *v)
{
    std::array<quint8, 1> data;
    qint64 read = io->read(reinterpret_cast<char *>(data.data()), 1);
    if (read != 1)
        return false;
    *v = data[0];
    return true;
}

inline bool psdreadLE(QIODevice *io, quint8 *v)
{
    std::array<quint8, 1> data;
    qint64 read = io->read(reinterpret_cast<char *>(data.data()), 1);
    if (read != 1)
        return false;
    *v = data[0];
    return true;
}

inline bool psdreadBE(QIODevice *io, quint16 *v)
{
    std::array<quint8, sizeof(quint16) / sizeof(quint8)> data;
    qint64 read = io->read(reinterpret_cast<char *>(data.data()), 2);
    if (read != 2)
        return false;
    *v = quint16((quint16(data[0]) << 8U) | data[1]);
    return true;
}

inline bool psdreadLE(QIODevice *io, quint16 *v)
{
    std::array<quint8, sizeof(quint16) / sizeof(quint8)> data;
    qint64 read = io->read(reinterpret_cast<char *>(data.data()), 2);
    if (read != 2)
        return false;
    *v = quint16((quint16(data[1]) << 8U) | data[0]);
    return true;
}

inline bool psdreadBE(QIODevice *io, quint32 *v)
{
    std::array<quint8, sizeof(quint32) / sizeof(quint8)> data;
    qint64 read = io->read(reinterpret_cast<char *>(data.data()), 4);
    if (read != 4)
        return false;
    *v = (quint32(data[0]) << 24U) | (quint32(data[1]) << 16U) | (quint32(data[2]) << 8U) | data[3];
    return true;
}

inline bool psdreadLE(QIODevice *io, quint32 *v)
{
    std::array<quint8, sizeof(quint32) / sizeof(quint8)> data;
    qint64 read = io->read(reinterpret_cast<char *>(data.data()), 4);
    if (read != 4)
        return false;
    *v = (quint32(data[3]) << 24U) | (quint32(data[2]) << 16U) | (quint32(data[1]) << 8U) | data[0];
    return true;
}

inline bool psdreadBE(QIODevice *io, quint64 *v)
{
    std::array<quint8, sizeof(quint64) / sizeof(quint8)> data;
    qint64 read = io->read(reinterpret_cast<char *>(data.data()), 8);
    if (read != 8)
        return false;
    *v = (quint64(data[0]) << 56U) | (quint64(data[1]) << 48U) | (quint64(data[2]) << 40U) | (quint64(data[3]) << 32U) | (quint64(data[4]) << 24U)
        | (quint64(data[5]) << 16U) | (quint64(data[6]) << 8U) | data[7];
    return true;
}

inline bool psdreadLE(QIODevice *io, quint64 *v)
{
    std::array<quint8, sizeof(quint64) / sizeof(quint8)> data;
    qint64 read = io->read(reinterpret_cast<char *>(data.data()), 8);
    if (read != 8)
        return false;
    *v = (quint64(data[7]) << 56U) | (quint64(data[6]) << 48U) | (quint64(data[5]) << 40U) | (quint64(data[4]) << 32U) | (quint64(data[3]) << 24U)
        | (quint64(data[2]) << 16U) | (quint64(data[1]) << 8U) | data[0];
    return true;
}

/**
 * Templated reading fallbacks for non-integral types.
 */

template<typename T>
inline bool psdreadBE(QIODevice *io, std::enable_if_t<sizeof(T) == sizeof(quint8), T *> v)
{
    return psdreadBE(io, reinterpret_cast<quint8 *>(v));
}

template<typename T>
inline bool psdreadBE(QIODevice *io, std::enable_if_t<sizeof(T) == sizeof(quint16), T *> v)
{
    return psdreadBE(io, reinterpret_cast<quint16 *>(v));
}

template<typename T>
inline bool psdreadBE(QIODevice *io, std::enable_if_t<sizeof(T) == sizeof(quint32), T *> v)
{
    return psdreadBE(io, reinterpret_cast<quint32 *>(v));
}

template<typename T>
inline bool psdreadBE(QIODevice *io, std::enable_if_t<sizeof(T) == sizeof(quint64), T *> v)
{
    return psdreadBE(io, reinterpret_cast<quint64 *>(v));
}

template<typename T>
inline bool psdreadLE(QIODevice *io, std::enable_if_t<sizeof(T) == sizeof(quint8), T *> v)
{
    return psdreadLE(io, reinterpret_cast<quint8 *>(v));
}

template<typename T>
inline bool psdreadLE(QIODevice *io, std::enable_if_t<sizeof(T) == sizeof(quint16), T *> v)
{
    return psdreadLE(io, reinterpret_cast<quint16 *>(v));
}

template<typename T>
inline bool psdreadLE(QIODevice *io, std::enable_if_t<sizeof(T) == sizeof(quint32), T *> v)
{
    return psdreadLE(io, reinterpret_cast<quint32 *>(v));
}

template<typename T>
inline bool psdreadLE(QIODevice *io, std::enable_if_t<sizeof(T) == sizeof(quint64), T *> v)
{
    return psdreadLE(io, reinterpret_cast<quint64 *>(v));
}

template<psd_byte_order byteOrder = psd_byte_order::psdBigEndian, typename T>
inline bool psdread(QIODevice *io, T *v)
{
    if (byteOrder == psd_byte_order::psdLittleEndian) {
        return psdreadLE<T>(io, v);
    } else {
        return psdreadBE<T>(io, v);
    }
}

template<psd_byte_order byteOrder = psd_byte_order::psdBigEndian>
inline QByteArray psdreadBytes(QIODevice *io, qint64 v)
{
    QByteArray b = io->read(v);
    if (byteOrder == psd_byte_order::psdLittleEndian) {
        std::reverse(b.begin(), b.end());
    }
    return b;
}

template<psd_byte_order byteOrder = psd_byte_order::psdBigEndian>
inline bool psdread_pascalstring(QIODevice *io, QString &s, qint64 padding)
{
    quint8 length;
    if (!psdread<byteOrder>(io, &length)) {
        return false;
    }

    if (length == 0) {
        // read the padding
        for (qint64 i = 0; i < padding - 1; ++i) {
            io->seek(io->pos() + 1);
        }
        return true;
    }

    QByteArray chars = io->read(length);
    if (chars.length() != length) {
        return false;
    }

    // read padding byte
    quint32 paddedLength = length + 1;
    if (padding > 0) {
        while (paddedLength % padding != 0) {
            if (!io->seek(io->pos() + 1)) {
                return false;
            }
            paddedLength++;
        }
    }

    s.append(QString::fromLatin1(chars));

    return true;
}

template<psd_byte_order byteOrder = psd_byte_order::psdBigEndian>
inline bool psdread_unicodestring(QIODevice *io, QString &s)
{
    quint32 stringlen;
    if (!psdread<byteOrder>(io, &stringlen)) {
        return false;
    }

    s.reserve(static_cast<int>(stringlen));

    for (quint32 i = 0; i < stringlen; ++i) {
        quint16 ch(0);
        if (!psdread<byteOrder>(io, &ch)) {
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
inline bool psd_read_blendmode(QIODevice *io, QString &blendModeKey)
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
