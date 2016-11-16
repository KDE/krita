/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef KOCLIPMASK_H
#define KOCLIPMASK_H

#include "kritaflake_export.h"

#include <QScopedPointer>
#include <QList>

class KoShape;
class QRectF;
class QTransform;
class QPainter;


class KRITAFLAKE_EXPORT KoClipMask
{
public:
    enum CoordinateSystem {
        UserSpaceOnUse,
        ObjectBoundingBox
    };

public:
    KoClipMask();
    ~KoClipMask();

    KoClipMask *clone() const;

    CoordinateSystem coordinates() const;
    void setCoordinates(CoordinateSystem value);

    CoordinateSystem contentCoordinates() const;
    void setContentCoordinates(CoordinateSystem value);

    QRectF maskRect() const;
    void setMaskRect(const QRectF &value);

    QList<KoShape *> shapes() const;
    void setShapes(const QList<KoShape *> &value);

    bool isEmpty() const;

    QTransform extraShapeTransform() const;
    void setExtraShapeTransform(const QTransform &value);

    void drawMask(QPainter *painter, KoShape *shape);

private:
    KoClipMask(const KoClipMask &rhs);

    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KOCLIPMASK_H
