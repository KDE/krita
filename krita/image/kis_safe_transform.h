/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_SAFE_TRANSFORM_H
#define __KIS_SAFE_TRANSFORM_H

#include <QScopedPointer>

#include "kritaimage_export.h"

class QTransform;
class QRect;
class QRectF;
class QPolygonF;


class KRITAIMAGE_EXPORT KisSafeTransform
{
public:
    KisSafeTransform(const QTransform &transform,
                     const QRect &bounds,
                     const QRect &srcInterestRect);

    ~KisSafeTransform();

    QPolygonF srcClipPolygon() const;
    QPolygonF dstClipPolygon() const;

    QPolygonF mapForward(const QPolygonF &p);
    QPolygonF mapBackward(const QPolygonF &p);

    QRectF mapRectForward(const QRectF &rc);
    QRectF mapRectBackward(const QRectF &rc);

    QRect mapRectForward(const QRect &rc);
    QRect mapRectBackward(const QRect &rc);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_SAFE_TRANSFORM_H */
