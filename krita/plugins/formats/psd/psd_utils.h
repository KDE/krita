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
#ifndef PSD_UTILS_H
#define PSD_UTILS_H

#include <QtGlobal>

class QIODevice;
class QString;

bool psdwrite(QIODevice* io, quint8 v);
bool psdwrite(QIODevice* io, quint16 v);
bool psdwrite(QIODevice* io, qint16 v);
bool psdwrite(QIODevice* io, quint32 v);
bool psdwrite(QIODevice* io, const QString &s);
bool psdwrite_pascalstring(QIODevice* io, const QString &s);
bool psdpad(QIODevice* io, quint32 padding);

bool psdread(QIODevice* io, quint8* v);
bool psdread(QIODevice* io, quint16* v);
bool psdread(QIODevice* io, qint16* v);
bool psdread(QIODevice* io, quint32* v);
bool psdread(QIODevice* io, quint64* v);
bool psdread_pascalstring(QIODevice* io, QString& s);

#endif // PSD_UTILS_H
