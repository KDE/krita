/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_marker_painter.h"

#include <KoColor.h>
#include <KoColorSpace.h>

#include "kis_paint_device.h"

#include "kis_algebra_2d.h"
#include "kis_sequential_iterator.h"


struct KisMarkerPainter::Private
{
    Private(KisPaintDeviceSP _device, const KoColor &_color) : device(_device), color(_color) {}

    KisPaintDeviceSP device;
    const KoColor &color;
};

KisMarkerPainter::KisMarkerPainter(KisPaintDeviceSP device, const KoColor &color)
    : m_d(new Private(device, color))
{
}

KisMarkerPainter::~KisMarkerPainter()
{
}



bool KisMarkerPainter::isNumberInValidRange(qint32 number)
{
    if (number < -ValidNumberRangeValue || number > ValidNumberRangeValue)
        return false;
    return true;
}

bool KisMarkerPainter::isRectInValidRange(const QRect &rect)
{
    return isNumberInValidRange(rect.x())
            && isNumberInValidRange(rect.y())
            && isNumberInValidRange(rect.width())
            && isNumberInValidRange(rect.height());
}

void KisMarkerPainter::fillHalfBrushDiff(const QPointF &p1, const QPointF &p2, const QPointF &p3,
                                         const QPointF &center, qreal radius)
{
    KoColor currentColor(m_d->color);

    const int pixelSize = m_d->device->pixelSize();
    const KoColorSpace *cs = m_d->device->colorSpace();

    const qreal fadedRadius = radius + 1;
    QRectF boundRect(center.x() - fadedRadius, center.y() - fadedRadius,
                     2 * fadedRadius, 2 * fadedRadius);

    KisAlgebra2D::RightHalfPlane plane1(p1, p2);
    KisAlgebra2D::RightHalfPlane plane2(p2, p3);
    KisAlgebra2D::OuterCircle outer(center, radius);

    boundRect = KisAlgebra2D::cutOffRect(boundRect, plane1);
    boundRect = KisAlgebra2D::cutOffRect(boundRect, plane2);

    QRect alignedRect = boundRect.toAlignedRect();

    KIS_SAFE_ASSERT_RECOVER_RETURN(isRectInValidRange(alignedRect));

    KisSequentialIterator it(m_d->device, alignedRect);

    while (it.nextPixel()) {
        QPoint pt(it.x(), it.y());

        qreal value1 = plane1.value(pt);
        if (value1 < 0) continue;

        qreal value2 = plane2.value(pt);
        if (value2 < 0) continue;

        qreal value3 = outer.fadeSq(pt);
        if (value3 > 1.0) continue;

        // qreal fadePos =
        //     value1 < 0 || value2 < 0 ?
        //     qMax(-value1, -value2) : value3;
        qreal fadePos = value3;

        const quint8 srcAlpha = fadePos > 0 ? quint8((1.0 - fadePos) * 255.0) : 255;
        const quint8 dstAlpha = cs->opacityU8(it.rawData());

        if (srcAlpha > dstAlpha) {
            currentColor.setOpacity(srcAlpha);
            memcpy(it.rawData(), currentColor.data(), pixelSize);
        }
    }
}

void KisMarkerPainter::fillFullCircle(const QPointF &center, qreal radius)
{
    KoColor currentColor(m_d->color);

    const int pixelSize = m_d->device->pixelSize();
    const KoColorSpace *cs = m_d->device->colorSpace();

    const qreal fadedRadius = radius + 1;
    QRectF boundRect(center.x() - fadedRadius, center.y() - fadedRadius,
                     2 * fadedRadius, 2 * fadedRadius);

    KisAlgebra2D::OuterCircle outer(center, radius);

    QRect alignedRect = boundRect.toAlignedRect();

    KIS_SAFE_ASSERT_RECOVER_RETURN(isRectInValidRange(alignedRect));

    KisSequentialIterator it(m_d->device, alignedRect);
    while (it.nextPixel()) {
        QPoint pt(it.x(), it.y());

        qreal value3 = outer.fadeSq(pt);
        if (value3 > 1.0) continue;

        const quint8 srcAlpha = value3 > 0 ? quint8((1.0 - value3) * 255.0) : 255;
        const quint8 dstAlpha = cs->opacityU8(it.rawData());

        if (srcAlpha > dstAlpha) {
            currentColor.setOpacity(srcAlpha);
            memcpy(it.rawData(), currentColor.data(), pixelSize);
        }
    }
}

void KisMarkerPainter::fillCirclesDiff(const QPointF &c1, qreal r1,
                                       const QPointF &c2, qreal r2)
{
    QVector<QPointF> n = KisAlgebra2D::intersectTwoCircles(c1, r1, c2, r2);

    if (n.size() < 2) {
        fillFullCircle(c2, r2);
    } else {
        const QPointF diff = c2 - c1;
        const qreal normDiffInv = 1.0 / KisAlgebra2D::norm(diff);
        const QPointF direction = diff * normDiffInv;
        const QPointF p = c1 + r1 * direction;
        const QPointF q = c2 + r2 * direction;

        fillHalfBrushDiff(n[0], p, q, c2, r2);
        fillHalfBrushDiff(q, p, n[1], c2, r2);
    }
}

