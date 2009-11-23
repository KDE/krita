/*
 *  Copyright (c) 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "psd_utils.h"

#include <QIODevice>
#include <QString>

#include <netinet/in.h> // htonl

#include <kis_debug.h>

bool psdwrite(QIODevice* io, quint8 v)
{
    int written = io->write((char*)&v, 1);
    return written == 1;
}

bool psdwrite(QIODevice* io, quint16 v)
{
    quint16 val = ntohs(v);
    int written = io->write((char*)&val, 2);
    return written == 2;
}

bool psdwrite(QIODevice* io, qint16 v)
{
    qint16 val = ntohs(v);
    int written = io->write((char*)&val, 2);
    return written == 2;
}


bool psdwrite(QIODevice* io, quint32 v)
{
    quint32 val = ntohl(v);
    int written = io->write((char*)&val, 4);
    return written == 4;
}

bool psdpad(QIODevice* io, quint32 padding)
{
    char* pad = new char(padding);
    memset(pad, 0, padding);
    quint32 written = io->write(pad, padding);
    return written == padding;
}

bool psdwrite(QIODevice* io, const QString &s)
{
    int l = s.length();
    char* str = s.toAscii().data();
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

    char* str = s.toAscii().data();
    int written = io->write(str, length);
    if (written != length) return false;

    if ((length & 0x01) != 0) {
        return psdwrite(io, (quint8)0);
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
    *v = ntohs(val);
    return true;
}

bool psdread(QIODevice* io, qint16* v)
{
    qint16 val;
    quint64 read = io->read((char*)&val, 2);
    if (read != 2) return false;
    *v = ntohs(val);
    return true;
}


bool psdread(QIODevice* io, quint32* v)
{
    quint32 val;
    quint64 read = io->read((char*)&val, 4);
    if (read != 4) return false;
    *v = ntohl(val);
    return true;
}

bool psdread(QIODevice* io, quint64* v)
{
    quint64 val;
    quint64 read = io->read((char*)&val, 8);
    if (read != 8) return false;
    *v = ntohl(val);
    return true;
}

bool psdread_pascalstring(QIODevice* io, QString& s)
{

    quint8 length;
    if (!psdread(io, &length)) return false;

    if (length == 0) {
        // read another null byte
        if (!psdread(io, &length)) return false;
        return (length == 0);
    }

    QByteArray chars = io->read(length);
    if (chars.length() != length) return false;

    s.append(chars);

    // read padding byte
    if ((length & 0x01) != 0) {
        if (!psdread(io, &length)) return false;
        return (length == 0);
    }

    return true;
}

