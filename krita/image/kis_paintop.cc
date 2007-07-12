/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Clarence Dang <dang@kde.org>
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2004,2007 Cyrille Berger <cberger@cberger.net>
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

#include <QWidget>

#include <QString>

#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoPointerEvent.h>

#include "kis_painter.h"
#include "kis_layer.h"
#include "kis_brush.h"
#include "kis_types.h"
#include "kis_paintop.h"
#include "kis_qimage_mask.h"

#include "kis_global.h"
#include "kis_iterators_pixel.h"
#include "kis_datamanager.h"


#define BEZIER_FLATNESS_THRESHOLD 0.5

struct KisPaintOp::Private
{
    Private() : dab(0) {}
    KisPaintDeviceSP dab;
    KoColor color;
    KoColor previousPaintColor;
    KisQImagemaskSP previousMask;
    KisPainter * painter;
    KisPaintDeviceSP source; // use this layer as source layer for the operation
};


KisPaintOp::KisPaintOp( KisPainter * painter) : d(new Private)
{
    d->painter = painter;
    setSource(painter->device());
}

KisPaintOp::~KisPaintOp()
{
    delete d;
}

KisPaintDeviceSP KisPaintOp::computeDab(KisQImagemaskSP mask) {
    return computeDab(mask, d->painter->device()->colorSpace());
}

KisPaintDeviceSP KisPaintOp::computeDab(KisQImagemaskSP mask, KoColorSpace *cs)
{
    // XXX: According to the SeaShore source, the Gimp uses a
    // temporary layer the size of the layer that is being painted
    // on. This layer is cleared between painting actions. Our
    // temporary layer, dab, is for every paintAt, composited with
    // the target layer. We only use a real temporary layer for things
    // like filter tools -- and for indirect painting, it turns out.


    qint32 maskWidth = mask->width();
    qint32 maskHeight = mask->height();

    if( !d->dab or d->dab->colorSpace() != cs or not( d->previousPaintColor == d->painter->paintColor() ) ) {
        d->dab = KisPaintDeviceSP(new KisPaintDevice(cs, "dab"));
        d->color = d->painter->paintColor();
        d->previousPaintColor = d->painter->paintColor();
        d->color.convertTo(cs);
        d->color.fromKoColor( d->painter->paintColor());
        d->dab->dataManager()->setDefaultPixel( d->color.data() );
    } else if(d->previousMask == mask) {
        return d->dab;
    }
    d->previousMask = mask;
    
    // Convert the kiscolor to the right colorspace. TODO: check if the paintColor has change
    Q_CHECK_PTR(d->dab);

    quint8 * maskData = mask->data();

    // Apply the alpha mask
    KisHLineIteratorPixel hiter = d->dab->createHLineIterator(0, 0, maskWidth);
    for (int y = 0; y < maskHeight; y++)
    {
        while(! hiter.isDone())
        {
            int hiterConseq = hiter.nConseqHPixels();
            cs->setAlpha( hiter.rawData(), OPACITY_OPAQUE, hiterConseq );
            cs->applyAlphaU8Mask( hiter.rawData(), maskData, hiterConseq);
            hiter += hiterConseq;
            maskData += hiterConseq;
        }
        hiter.nextRow();
    }

    return d->dab;
}

void KisPaintOp::splitCoordinate(double coordinate, qint32 *whole, double *fraction)
{
    qint32 i = static_cast<qint32>(coordinate);

    if (coordinate < 0) {
        // We always want the fractional part to be positive.
        // E.g. -1.25 becomes -2 and +0.75
        i--;
    }

    double f = coordinate - i;

    *whole = i;
    *fraction = f;
}

void KisPaintOp::setSource(KisPaintDeviceSP p) {
    Q_ASSERT(p);
    d->source = p;
}


double KisPaintOp::paintBezierCurve(const KisPaintInformation &pi1,
                                    const QPointF &control1,
                                    const QPointF &control2,
                                    const KisPaintInformation &pi2,
                                    const double savedDist)
{
    double newDistance;
    double d1 = KisPainter::pointToLineDistance(control1, pi1.pos, pi2.pos);
    double d2 = KisPainter::pointToLineDistance(control2, pi1.pos, pi2.pos);

    if (d1 < BEZIER_FLATNESS_THRESHOLD && d2 < BEZIER_FLATNESS_THRESHOLD) {
        newDistance = paintLine(pi1, pi2, savedDist);
    } else {
        // Midpoint subdivision. See Foley & Van Dam Computer Graphics P.508
        KisVector2D p1 = pi1.pos;
        KisVector2D p2 = control1;
        KisVector2D p3 = control2;
        KisVector2D p4 = pi2.pos;

        KisVector2D l2 = (p1 + p2) / 2;
        KisVector2D h = (p2 + p3) / 2;
        KisVector2D l3 = (l2 + h) / 2;
        KisVector2D r3 = (p3 + p4) / 2;
        KisVector2D r2 = (h + r3) / 2;
        KisVector2D l4 = (l3 + r2) / 2;

        double midPressure = (pi1.pressure + pi2.pressure) / 2;
        double midXTilt = (pi1.xTilt + pi2.xTilt) / 2;
        double midYTilt = (pi1.yTilt + pi2.yTilt) / 2;

        KisPaintInformation middlePI( l4.toKoPoint(), midPressure, midXTilt, midYTilt );
        newDistance = paintBezierCurve( pi1,
                                       l2.toKoPoint(), l3.toKoPoint(),
                                       middlePI, savedDist);
        newDistance = paintBezierCurve(middlePI,
                                       r2.toKoPoint(),
                                       r3.toKoPoint(),
                                       pi2, newDistance);
    }

    return newDistance;
}


double KisPaintOp::paintLine(const KisPaintInformation &pi1,
                     const KisPaintInformation &pi2,
                     double savedDist)
{
    KisVector2D end(pi2.pos);
    KisVector2D start(pi1.pos);

    KisVector2D dragVec = end - start;
    KisVector2D movement = dragVec;

    if (savedDist < 0) {
        paintAt(pi1);
        savedDist = 0;
    }

    // XXX: The spacing should vary as the pressure changes along the line.
    // This is a quick simplification.
    double xSpacing = d->painter->brush()->xSpacing((pi1.pressure + pi2.pressure) / 2);
    double ySpacing = d->painter->brush()->ySpacing((pi1.pressure + pi2.pressure) / 2);

    if (xSpacing < 0.5) {
        xSpacing = 0.5;
    }
    if (ySpacing < 0.5) {
        ySpacing = 0.5;
    }

    double xScale = 1;
    double yScale = 1;
    double spacing;
    // Scale x or y so that we effectively have a square brush
    // and calculate distance in that coordinate space. We reverse this scaling
    // before drawing the brush. This produces the correct spacing in both
    // x and y directions, even if the brush's aspect ratio is not 1:1.
    if (xSpacing > ySpacing) {
        yScale = xSpacing / ySpacing;
        spacing = xSpacing;
    }
    else {
        xScale = ySpacing / xSpacing;
        spacing = ySpacing;
    }

    dragVec.setX(dragVec.x() * xScale);
    dragVec.setY(dragVec.y() * yScale);

    double newDist = dragVec.length();
    double dist = savedDist + newDist;
    double l_savedDist = savedDist;

    if (dist < spacing) {
        return dist;
    }

    dragVec.normalize();
    KisVector2D step(0, 0);

    while (dist >= spacing) {
        if (l_savedDist > 0) {
            step += dragVec * (spacing - l_savedDist);
            l_savedDist -= spacing;
        }
        else {
            step += dragVec * spacing;
        }

        QPointF p(start.x() + (step.x() / xScale), start.y() + (step.y() / yScale));

        double distanceMoved = step.length();
        double t = 0;

        if (newDist > DBL_EPSILON) {
            t = distanceMoved / newDist;
        }

        double pressure = (1 - t) * pi1.pressure + t * pi2.pressure;
        double xTilt = (1 - t) * pi1.xTilt + t * pi2.xTilt;
        double yTilt = (1 - t) * pi1.yTilt + t * pi2.yTilt;

        paintAt(KisPaintInformation(p, pressure, xTilt, yTilt, movement));
        dist -= spacing;
    }

    d->painter->addDirtyRect( QRect( pi1.pos.toPoint(), pi2.pos.toPoint() ) );

    if (dist > 0)
        return dist;
    else
        return 0;
}

KisPainter* KisPaintOp::painter()
{
    return d->painter;
}

KisPaintDeviceSP KisPaintOp::source()
{
    return d->source;
}

void KisPaintOpSettings::mousePressEvent(KoPointerEvent *e)
{
  e->ignore();
}

void KisPaintOpSettings::activate()
{
}

KisPaintOpSettings* KisPaintOpFactory::settings(QWidget* /*parent*/, const KoInputDevice& /*inputDevice*/, KisImageSP /*image*/) { return 0; }

QString KisPaintOpFactory::pixmap()
{
    return "";
}

bool KisPaintOpFactory::userVisible(KoColorSpace * cs )
{
    return cs && cs->id() != "WET";
}
