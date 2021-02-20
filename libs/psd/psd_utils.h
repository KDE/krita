/*
 *  SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef PSD_UTILS_H
#define PSD_UTILS_H

#include <QtGlobal>

#include <resources/KoPattern.h>

#include "kritapsd_export.h"


class QIODevice;
class QString;

bool KRITAPSD_EXPORT psdwrite(QIODevice* io, quint8 v);
bool KRITAPSD_EXPORT psdwrite(QIODevice* io, quint16 v);
bool KRITAPSD_EXPORT psdwrite(QIODevice* io, qint16 v);
bool KRITAPSD_EXPORT psdwrite(QIODevice* io, quint32 v);
bool KRITAPSD_EXPORT psdwrite(QIODevice* io, const QString &s);
bool KRITAPSD_EXPORT psdwrite(QIODevice* io, double v);
bool KRITAPSD_EXPORT psdwrite_pascalstring(QIODevice* io, const QString &s);
bool KRITAPSD_EXPORT psdwrite_pascalstring(QIODevice* io, const QString &s, int padding);
bool KRITAPSD_EXPORT psdpad(QIODevice* io, quint32 padding);

bool KRITAPSD_EXPORT psdread(QIODevice* io, quint8* v);
bool KRITAPSD_EXPORT psdread(QIODevice* io, quint16* v);
bool KRITAPSD_EXPORT psdread(QIODevice* io, qint16* v);
bool KRITAPSD_EXPORT psdread(QIODevice* io, quint32* v);
bool KRITAPSD_EXPORT psdread(QIODevice* io, qint32* v);
bool KRITAPSD_EXPORT psdread(QIODevice* io, quint64* v);
bool KRITAPSD_EXPORT psdread(QIODevice* io, double* v);
bool KRITAPSD_EXPORT psdread_pascalstring(QIODevice* io, QString& s, int padding);
bool KRITAPSD_EXPORT psdread_unicodestring(QIODevice* io, QString &s);
bool KRITAPSD_EXPORT psd_read_blendmode(QIODevice* io, QString &blendModeKey);


#endif // PSD_UTILS_H
