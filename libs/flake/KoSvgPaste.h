/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KOSVGPASTE_H
#define KOSVGPASTE_H

#include "kritaflake_export.h"
#include <QList>

class KoShape;
class QRectF;
class QSizeF;
class QByteArray;

class KRITAFLAKE_EXPORT KoSvgPaste
{
public:
    static bool hasShapes();
    static QList<KoShape*> fetchShapes(const QRectF viewportInPx, qreal resolutionPPI, QSizeF *fragmentSize = 0);
    static QList<KoShape*> fetchShapesFromData(const QByteArray &data, const QRectF viewportInPx, qreal resolutionPPI, QSizeF *fragmentSize = 0);

};

#endif // KOSVGPASTE_H
