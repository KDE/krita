/*
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include <KoVcMultiArchBuildSupport.h> //MSVC requires that Vc come first

#include <cmath>

#include <QDomDocument>
#include <QVector>
#include <QPointF>

#include <kis_fast_math.h>
#include "kis_curve_rect_mask_generator.h"
#include "kis_cubic_curve.h"
#include "kis_antialiasing_fade_maker.h"


struct Q_DECL_HIDDEN KisCurveRectangleMaskGenerator::Private
{
    Private(bool enableAntialiasing)
        : fadeMaker(*this, enableAntialiasing)
    {
    }

    QVector<qreal> curveData;
    QList<QPointF> curvePoints;
    int curveResolution;
    bool dirty;

    qreal xcoeff;
    qreal ycoeff;

    KisAntialiasingFadeMaker2D<Private> fadeMaker;

    quint8 value(qreal xr, qreal yr) const;
};

KisCurveRectangleMaskGenerator::KisCurveRectangleMaskGenerator(qreal diameter, qreal ratio, qreal fh, qreal fv, int spikes, const KisCubicCurve &curve, bool antialiasEdges)
    : KisMaskGenerator(diameter, ratio, fh, fv, spikes, antialiasEdges, RECTANGLE, SoftId), d(new Private(antialiasEdges))
{
    d->curveResolution = qRound( qMax(width(),height()) * OVERSAMPLING);
    d->curveData = curve.floatTransfer( d->curveResolution + 1);
    d->curvePoints = curve.points();
    setCurveString(curve.toString());
    d->dirty = false;

    setScale(1.0, 1.0);
}

void KisCurveRectangleMaskGenerator::setScale(qreal scaleX, qreal scaleY)
{
    KisMaskGenerator::setScale(scaleX, scaleY);

    qreal halfWidth = 0.5 * effectiveSrcWidth();
    qreal halfHeight = 0.5 * effectiveSrcHeight();

    d->xcoeff = 1.0 / halfWidth;
    d->ycoeff = 1.0 / halfHeight;

    d->fadeMaker.setLimits(halfWidth, halfHeight);
}

KisCurveRectangleMaskGenerator::~KisCurveRectangleMaskGenerator()
{
    delete d;
}

quint8 KisCurveRectangleMaskGenerator::Private::value(qreal xr, qreal yr) const
{
    xr = qAbs(xr) * xcoeff;
    yr = qAbs(yr) * ycoeff;

    int sIndex = qRound(xr * (curveResolution));
    int tIndex = qRound(yr * (curveResolution));

    int sIndexInverted = curveResolution - sIndex;
    int tIndexInverted = curveResolution - tIndex;

    qreal blend = (curveData.at(sIndex) * (1.0 - curveData.at(sIndexInverted)) *
                   curveData.at(tIndex) * (1.0 - curveData.at(tIndexInverted)));

    return (1.0 - blend) * 255;
}

quint8 KisCurveRectangleMaskGenerator::valueAt(qreal x, qreal y) const
{

    if (KisMaskGenerator::d->empty) {
        return 255;
    }

    double xr = x;
    double yr = qAbs(y);

    if (KisMaskGenerator::d->spikes > 2) {
        double angle = (KisFastMath::atan2(yr, xr));

        while (angle > KisMaskGenerator::d->cachedSpikesAngle ){
            double sx = xr;
            double sy = yr;

            xr = KisMaskGenerator::d->cs * sx - KisMaskGenerator::d->ss * sy;
            yr = KisMaskGenerator::d->ss * sx + KisMaskGenerator::d->cs * sy;

            angle -= 2 * KisMaskGenerator::d->cachedSpikesAngle;
        }
    }

    quint8 value;
    if (d->fadeMaker.needFade(xr, yr, &value)) {
        return value;
    }

    return d->value(xr, yr);
}

void KisCurveRectangleMaskGenerator::toXML(QDomDocument& doc, QDomElement& e) const
{
    KisMaskGenerator::toXML(doc, e);
    e.setAttribute("softness_curve", curveString());
}

void KisCurveRectangleMaskGenerator::setSoftness(qreal softness)
{
    // performance
    if (!d->dirty && softness == 1.0) return;
    d->dirty = true;
    KisMaskGenerator::setSoftness(softness);
    KisCurveCircleMaskGenerator::transformCurveForSoftness(softness,d->curvePoints, d->curveResolution + 1, d->curveData);
    d->dirty = false;
}

