/*
 *  SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef PSD_UTILS_H
#define PSD_UTILS_H

#include <QtGlobal>

#include <resources/KoPattern.h>

#include "kritapsdutils_export.h"

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

bool KRITAPSDUTILS_EXPORT psdread(QIODevice *io, quint8 *v);
bool KRITAPSDUTILS_EXPORT psdread(QIODevice *io, quint16 *v);
bool KRITAPSDUTILS_EXPORT psdread(QIODevice *io, qint16 *v);
bool KRITAPSDUTILS_EXPORT psdread(QIODevice *io, quint32 *v);
bool KRITAPSDUTILS_EXPORT psdread(QIODevice *io, qint32 *v);
bool KRITAPSDUTILS_EXPORT psdread(QIODevice *io, quint64 *v);
bool KRITAPSDUTILS_EXPORT psdread(QIODevice *io, double *v);
bool KRITAPSDUTILS_EXPORT psdread_pascalstring(QIODevice *io, QString &s, int padding);
bool KRITAPSDUTILS_EXPORT psdread_unicodestring(QIODevice *io, QString &s);
bool KRITAPSDUTILS_EXPORT psd_read_blendmode(QIODevice *io, QString &blendModeKey);

#endif // PSD_UTILS_H
