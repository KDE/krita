/*
 *  SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "psd_utils.h"

#include <QtEndian>

#include <QIODevice>
#include <QString>

#include "psd.h"

bool psdwrite(QIODevice* io, quint8 v)
{
    int written = io->write((char*)&v, 1);
    return written == 1;
}

bool psdwrite(QIODevice* io, quint16 v)
{
    quint16 val = qToBigEndian((quint16)v);
    int written = io->write((char*)&val, 2);
    return written == 2;
}

bool psdwrite(QIODevice* io, qint16 v)
{
    qint16 val = qToBigEndian((qint16)v);
    int written = io->write((char*)&val, 2);
    return written == 2;
}

bool psdwrite(QIODevice* io, quint32 v)
{
    quint32 val = qToBigEndian((quint32)v);
    int written = io->write((char*)&val, 4);
    return written == 4;
}

bool psdwrite(QIODevice* io, double val)
{
    Q_ASSERT(sizeof(double) == sizeof(qint64));
    void *v = &val;
    qint64 i = qToBigEndian<qint64>(*(qint64*)(v));
    quint64 write = io->write((char*)&i, 8);
    if (write != 8) return false;
    return true;
}

bool psdpad(QIODevice* io, quint32 padding)
{
    char* pad = new char[padding];
    memset(pad, 0, padding);
    quint32 written = io->write(pad, padding);
    delete [] pad;
    return written == padding;
}

bool psdwrite(QIODevice* io, const QString &s)
{
    int l = s.length();
    QByteArray b = s.toLatin1();
    char* str = b.data();
    int written = io->write(str, l);
    return written == l;
}

bool psdwrite_pascalstring(QIODevice* io, const QString &s)
{
    Q_ASSERT(s.length() < 256);
    Q_ASSERT(s.length() >= 0);
    if (s.length() < 0 || s.length() > 255) return false;

    if (s.isNull()) {
        psdwrite(io, (quint8)0);
        psdwrite(io, (quint8)0);
        return true;
    }
    quint8 length = s.length();
    psdwrite(io, length);

    QByteArray b = s.toLatin1();
    char* str = b.data();
    int written = io->write(str, length);
    if (written != length) return false;

    if ((length & 0x01) != 0) {
        return psdwrite(io, (quint8)0);
    }

    return true;
}

bool psdwrite_pascalstring(QIODevice* io, const QString &s, int padding)
{
    Q_ASSERT(s.length() < 256);
    Q_ASSERT(s.length() >= 0);
    if (s.length() < 0 || s.length() > 255) return false;

    if (s.isNull()) {
        psdwrite(io, (quint8)0);
        psdwrite(io, (quint8)0);
        return true;
    }
    quint8 length = s.length();
    psdwrite(io, length);

    QByteArray b = s.toLatin1();
    char* str = b.data();
    int written = io->write(str, length);
    if (written != length) return false;

    // If the total length (length byte + content) is not a multiple of padding, add zeroes to pad
    length++;
    if ((length % padding) != 0) {
        for (int i = 0; i < (padding - (length %padding)); i++) {
            psdwrite(io, (quint8)0);
        }
    }

    return true;
}


bool psdread(QIODevice *io, quint8 *v)
{
    quint64 read = io->read((char*)v, 1);
    if (read != 1) return false;
    return true;
}

bool psdread(QIODevice* io, quint16* v)
{
    quint16 val;
    quint64 read = io->read((char*)&val, 2);
    if (read != 2) return false;
    *v = qFromBigEndian(val);
    return true;
}

bool psdread(QIODevice* io, qint16* v)
{
    qint16 val;
    quint64 read = io->read((char*)&val, 2);
    if (read != 2) return false;
    *v = qFromBigEndian(val);
    return true;
}


bool psdread(QIODevice* io, quint32* v)
{
    quint32 val;
    quint64 read = io->read((char*)&val, 4);
    if (read != 4) return false;
    *v = qFromBigEndian(val);
    return true;
}


bool psdread(QIODevice* io, qint32* v)
{
    qint32 val;
    quint64 read = io->read((char*)&val, 4);
    if (read != 4) return false;
    *v = qFromBigEndian(val);
    return true;
}

bool psdread(QIODevice* io, quint64* v)
{
    quint64 val;
    quint64 read = io->read((char*)&val, 8);
    if (read != 8) return false;

    *v = qFromBigEndian<qint64>((quint8*)&val);
    return true;
}

bool psdread(QIODevice* io, double* v)
{
    Q_ASSERT(sizeof(double) == sizeof(qint64));

    qint64 val;
    quint64 read = io->read((char*)&val, 8);
    if (read != 8) return false;
    *(reinterpret_cast<qint64*>(v)) = qFromBigEndian<qint64>(val);
    return true;
}

bool psdread_pascalstring(QIODevice* io, QString& s, int padding)
{
    quint8 length;
    if (!psdread(io, &length)) {
        return false;
    }

    if (length == 0) {
        // read the padding
        for (int i = 0; i < padding -1; ++i) {
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



bool psd_read_blendmode(QIODevice *io, QString &blendModeKey)
{
    QByteArray b;
    b = io->read(4);
    if(b.size() != 4 || QString(b) != "8BIM") {
        return false;
    }
    blendModeKey = QString(io->read(4));
    if (blendModeKey.size() != 4) {
        return false;
    }
    return true;
}




bool psdread_unicodestring(QIODevice *io, QString &s)
{
    quint32 stringlen;
    if (!psdread(io, &stringlen)) {
        return false;
    }

    for (uint i = 0; i < stringlen; ++i) {
        quint16 ch;
        if (!psdread(io, &ch)) {
            return false;
        }

        if (ch != 0) {
            QChar uch(ch);
            s.append(uch);
        }
    }

    return true;
}
