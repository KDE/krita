/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KOCLIPMASK_H
#define KOCLIPMASK_H

#include "kritaflake_export.h"

#include <KoFlakeCoordinateSystem.h>
#include <QList>
#include <QSharedDataPointer>

class KoShape;
class QRectF;
class QTransform;
class QPointF;
class QPainter;


class KRITAFLAKE_EXPORT KoClipMask
{
public:
    KoClipMask();
    ~KoClipMask();

    KoClipMask *clone() const;

    KoFlake::CoordinateSystem coordinates() const;
    void setCoordinates(KoFlake::CoordinateSystem value);

    KoFlake::CoordinateSystem contentCoordinates() const;
    void setContentCoordinates(KoFlake::CoordinateSystem value);

    QRectF maskRect() const;
    void setMaskRect(const QRectF &value);

    QList<KoShape *> shapes() const;
    void setShapes(const QList<KoShape *> &value);

    bool isEmpty() const;

    void setExtraShapeOffset(const QPointF &value);

    void drawMask(QPainter *painter, KoShape *shape);

private:
    struct Private;
    QSharedDataPointer<Private> m_d;
};

#endif // KOCLIPMASK_H
