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
    KoSvgPaste();
    virtual ~KoSvgPaste();


    bool hasShapes();
    QList<KoShape*> fetchShapes(QRectF viewportInPx, qreal resolutionPPI, QSizeF *fragmentSize = nullptr);
    static QList<KoShape*> fetchShapesFromData(const QByteArray &data, QRectF viewportInPx, qreal resolutionPPI, QSizeF *fragmentSize = nullptr);

private:
    class Private;
    Private *const d;

    Q_DISABLE_COPY(KoSvgPaste);
};

#endif // KOSVGPASTE_H
