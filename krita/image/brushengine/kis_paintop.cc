/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Clarence Dang <dang@kde.org>
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2004,2007,2010 Cyrille Berger <cberger@cberger.net>
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

#include "kis_paintop.h"

#include <math.h>

#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoPointerEvent.h>

#include "kis_painter.h"
#include "kis_layer.h"
#include "kis_types.h"

#include "kis_image.h"
#include "kis_paint_device.h"
#include "kis_global.h"
#include "kis_datamanager.h"
#include "kis_paintop_settings.h"
#include "kis_paintop_preset.h"
#include "kis_paint_information.h"
#include "kis_vec.h"
#include "kis_perspective_math.h"
#include "kis_fixed_paint_device.h"
#include "kis_paintop_utils.h"


#define BEZIER_FLATNESS_THRESHOLD 0.5
#include <kis_distance_information.h>

#ifdef Q_OS_WIN
#include <float.h>
#ifndef __MINGW32__
#define isnan _isnan
#endif
#endif

struct KisPaintOp::Private {
    Private(KisPaintOp *_q)
        : q(_q), dab(0),
          currentScale(1.0),
          currentRotation(0),
          fanCornersEnabled(false),
          fanCornersStep(1.0) {}

    KisPaintOp *q;

    KisFixedPaintDeviceSP dab;
    KisPainter* painter;
    qreal currentScale;
    qreal currentRotation;

    bool fanCornersEnabled;
    qreal fanCornersStep;
};


KisPaintOp::KisPaintOp(KisPainter * painter) : d(new Private(this))
{
    d->painter = painter;
}

KisPaintOp::~KisPaintOp()
{
    d->dab.clear();
    delete d;
}

KisFixedPaintDeviceSP KisPaintOp::cachedDab()
{
    return cachedDab(d->painter->device()->colorSpace());
}

KisFixedPaintDeviceSP KisPaintOp::cachedDab(const KoColorSpace *cs)
{
    if (!d->dab || !(*d->dab->colorSpace() == *cs)) {
        d->dab = new KisFixedPaintDevice(cs);
    }
    return d->dab;
}

void KisPaintOp::setFanCornersInfo(bool fanCornersEnabled, qreal fanCornersStep)
{
    d->fanCornersEnabled = fanCornersEnabled;
    d->fanCornersStep = fanCornersStep;
}

void KisPaintOp::splitCoordinate(qreal coordinate, qint32 *whole, qreal *fraction)
{
    const qint32 i = std::floor(coordinate);
    const qreal f = coordinate - i;

    *whole = i;
    *fraction = f;
}

static void paintBezierCurve(KisPaintOp *paintOp,
                             const KisPaintInformation &pi1,
                             const KisVector2D &control1,
                             const KisVector2D &control2,
                             const KisPaintInformation &pi2,
                             KisDistanceInformation *currentDistance)
{
    LineEquation line = LineEquation::Through(toKisVector2D(pi1.pos()), toKisVector2D(pi2.pos()));
    qreal d1 = line.absDistance(control1);
    qreal d2 = line.absDistance(control2);

    if ((d1 < BEZIER_FLATNESS_THRESHOLD && d2 < BEZIER_FLATNESS_THRESHOLD)
#ifdef Q_CC_MSVC
            || isnan(d1) || isnan(d2)) {
#else
            || std::isnan(d1) || std::isnan(d2)) {
#endif
        paintOp->paintLine(pi1, pi2, currentDistance);
    } else {
        // Midpoint subdivision. See Foley & Van Dam Computer Graphics P.508
        KisVector2D l2 = (toKisVector2D(pi1.pos()) + control1) / 2;
        KisVector2D h = (control1 + control2) / 2;
        KisVector2D l3 = (l2 + h) / 2;
        KisVector2D r3 = (control2 + toKisVector2D(pi2.pos())) / 2;
        KisVector2D r2 = (h + r3) / 2;
        KisVector2D l4 = (l3 + r2) / 2;

        KisPaintInformation middlePI = KisPaintInformation::mix(toQPointF(l4), 0.5, pi1, pi2);

        paintBezierCurve(paintOp, pi1, l2, l3, middlePI, currentDistance);
        paintBezierCurve(paintOp, middlePI, r2, r3, pi2, currentDistance);
    }
}

void KisPaintOp::paintBezierCurve(const KisPaintInformation &pi1,
                                  const QPointF &control1,
                                  const QPointF &control2,
                                  const KisPaintInformation &pi2,
                                  KisDistanceInformation *currentDistance)
{
    return ::paintBezierCurve(this, pi1, toKisVector2D(control1), toKisVector2D(control2), pi2, currentDistance);
}


void KisPaintOp::paintLine(const KisPaintInformation &pi1,
                           const KisPaintInformation &pi2,
                           KisDistanceInformation *currentDistance)
{
    KisPaintOpUtils::paintLine(*this, pi1, pi2, currentDistance,
                               d->fanCornersEnabled,
                               d->fanCornersStep);
}

void KisPaintOp::paintAt(const KisPaintInformation& info, KisDistanceInformation *currentDistance)
{
    Q_ASSERT(currentDistance);

    KisPaintInformation pi(info);
    pi.paintAt(*this, currentDistance);
}

KisPainter* KisPaintOp::painter() const
{
    return d->painter;
}

KisPaintDeviceSP KisPaintOp::source() const
{
    return d->painter->device();
}

qreal KisPaintOp::currentRotation() const
{
    return d->currentRotation;
}

qreal KisPaintOp::currentScale() const
{
    return d->currentScale;
}

void KisPaintOp::setCurrentRotation(qreal rotation)
{
    d->currentRotation = rotation;
}

void KisPaintOp::setCurrentScale(qreal scale)
{
    d->currentScale = scale;
}

