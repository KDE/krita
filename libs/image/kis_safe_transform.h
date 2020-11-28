/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
