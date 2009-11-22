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

bool psdwrite(QIODevice* io, quint16 v)
{
    quint16 val = ntohs(v);
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
    if (read != 4) return false;
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

bool psdreadpascalstring(QIODevice* io, QString& s)
{
    quint8 length;
    if (!psdread(io, &length)) return false;
    if (length < 1) return false;
}
