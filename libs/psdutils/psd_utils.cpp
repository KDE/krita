/*
 *  SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QIODevice>
#include <QString>
#include <QtEndian>

#include "psd_utils.h"

bool psdwrite(QIODevice *io, quint8 v)
{
    const qint64 written = io->write(reinterpret_cast<char *>(&v), 1);
    return written == 1;
}

bool psdwrite(QIODevice *io, quint16 v)
{
    quint16 val = qToBigEndian((quint16)v);
    int written = io->write((char *)&val, 2);
    return written == 2;
}

bool psdwrite(QIODevice *io, qint16 v)
{
    qint16 val = qToBigEndian((qint16)v);
    int written = io->write((char *)&val, 2);
    return written == 2;
}

bool psdwrite(QIODevice *io, quint32 v)
{
    quint32 val = qToBigEndian((quint32)v);
    int written = io->write((char *)&val, 4);
    return written == 4;
}

bool psdwrite(QIODevice *io, double val)
{
    Q_ASSERT(sizeof(double) == sizeof(qint64));
    void *v = &val;
    qint64 i = qToBigEndian<qint64>(*(qint64 *)(v));
    quint64 write = io->write((char *)&i, 8);
    if (write != 8)
        return false;
    return true;
}

bool psdpad(QIODevice *io, quint32 padding)
{
    char *pad = new char[padding];
    memset(pad, 0, padding);
    quint32 written = io->write(pad, padding);
    delete[] pad;
    return written == padding;
}

bool psdwrite(QIODevice *io, const QString &s)
{
    int l = s.length();
    QByteArray b = s.toLatin1();
    char *str = b.data();
    int written = io->write(str, l);
    return written == l;
}

bool psdwrite_pascalstring(QIODevice *io, const QString &s)
{
    Q_ASSERT(s.length() < 256);
    Q_ASSERT(s.length() >= 0);
    if (s.length() < 0 || s.length() > 255)
        return false;

    if (s.isNull()) {
        psdwrite(io, (quint8)0);
        psdwrite(io, (quint8)0);
        return true;
    }
    quint8 length = s.length();
    psdwrite(io, length);

    QByteArray b = s.toLatin1();
    char *str = b.data();
    int written = io->write(str, length);
    if (written != length)
        return false;

    if ((length & 0x01) != 0) {
        return psdwrite(io, (quint8)0);
    }

    return true;
}

bool psdwrite_pascalstring(QIODevice *io, const QString &s, int padding)
{
    Q_ASSERT(s.length() < 256);
    Q_ASSERT(s.length() >= 0);
    if (s.length() < 0 || s.length() > 255)
        return false;

    if (s.isNull()) {
        psdwrite(io, (quint8)0);
        psdwrite(io, (quint8)0);
        return true;
    }
    quint8 length = s.length();
    psdwrite(io, length);

    QByteArray b = s.toLatin1();
    char *str = b.data();
    int written = io->write(str, length);
    if (written != length)
        return false;

    // If the total length (length byte + content) is not a multiple of padding, add zeroes to pad
    length++;
    if ((length % padding) != 0) {
        for (int i = 0; i < (padding - (length % padding)); i++) {
            psdwrite(io, (quint8)0);
        }
    }

    return true;
}
