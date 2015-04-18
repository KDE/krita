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

#include <KoPattern.h>

#include "libkispsd_export.h"


class QIODevice;
class QString;

bool LIBKISPSD_EXPORT psdwrite(QIODevice* io, quint8 v);
bool LIBKISPSD_EXPORT psdwrite(QIODevice* io, quint16 v);
bool LIBKISPSD_EXPORT psdwrite(QIODevice* io, qint16 v);
bool LIBKISPSD_EXPORT psdwrite(QIODevice* io, quint32 v);
bool LIBKISPSD_EXPORT psdwrite(QIODevice* io, const QString &s);
bool LIBKISPSD_EXPORT psdwrite(QIODevice* io, double v);
bool LIBKISPSD_EXPORT psdwrite_pascalstring(QIODevice* io, const QString &s);
bool LIBKISPSD_EXPORT psdwrite_pascalstring(QIODevice* io, const QString &s, int padding);
bool LIBKISPSD_EXPORT psdpad(QIODevice* io, quint32 padding);

bool LIBKISPSD_EXPORT psdread(QIODevice* io, quint8* v);
bool LIBKISPSD_EXPORT psdread(QIODevice* io, quint16* v);
bool LIBKISPSD_EXPORT psdread(QIODevice* io, qint16* v);
bool LIBKISPSD_EXPORT psdread(QIODevice* io, quint32* v);
bool LIBKISPSD_EXPORT psdread(QIODevice* io, qint32* v);
bool LIBKISPSD_EXPORT psdread(QIODevice* io, quint64* v);
bool LIBKISPSD_EXPORT psdread(QIODevice* io, double* v);
bool LIBKISPSD_EXPORT psdread_pascalstring(QIODevice* io, QString& s, int padding);
bool LIBKISPSD_EXPORT psdread_unicodestring(QIODevice* io, QString &s);
bool LIBKISPSD_EXPORT psd_read_blendmode(QIODevice* io, QString &blendModeKey);


#endif // PSD_UTILS_H
