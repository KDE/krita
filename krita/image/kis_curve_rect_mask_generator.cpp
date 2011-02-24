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

#include <cmath>

#include <QDomDocument>
#include <QVector>
#include <QPointF>

#include <kis_fast_math.h>
#include "kis_curve_rect_mask_generator.h"
#include "kis_cubic_curve.h"

struct KisCurveRectangleMaskGenerator::Private {
    QVector<qreal> curveData;
    QList<QPointF> curvePoints;
    int curveResolution;
    bool dirty;
    qreal m_halfWidth, m_halfHeight;
};

KisCurveRectangleMaskGenerator::KisCurveRectangleMaskGenerator(qreal diameter, qreal ratio, qreal fh, qreal fv, int spikes, const KisCubicCurve &curve)
        : KisMaskGenerator(diameter, ratio, fh, fv, spikes, RECTANGLE, SoftId), d(new Private)
{
    d->curveResolution = qRound( qMax(width(),height()) * OVERSAMPLING);
    d->curveData = curve.floatTransfer( d->curveResolution + 1); 
    d->curvePoints = curve.points();
    setCurveString(curve.toString());
    d->dirty = false;
    d->m_halfWidth = KisMaskGenerator::d->diameter * 0.5;
    d->m_halfHeight = d->m_halfWidth * KisMaskGenerator::d->ratio;

}

KisCurveRectangleMaskGenerator::~KisCurveRectangleMaskGenerator()
{
    delete d;
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

    if(xr > d->m_halfWidth || xr < -d->m_halfWidth || yr > d->m_halfHeight || yr < -d->m_halfHeight) {
        return 255;
    }
    
    xr = qAbs(xr) / width();
    yr = qAbs(yr) / height();
    
    if (xr > 1.0 || yr > 1.0){
        return 255;
    }
    
    int sIndex = qRound(xr * (d->curveResolution));
    int tIndex = qRound(yr * (d->curveResolution));
    
    int sIndexInverted = d->curveResolution - sIndex;
    int tIndexInverted = d->curveResolution - tIndex;
    
    qreal blend = (d->curveData.at(sIndex) * (1.0 - d->curveData.at(sIndexInverted)) *
                  d->curveData.at(tIndex) * (1.0 - d->curveData.at(tIndexInverted)));
    
    return (1.0 - blend) * 255;
}

void KisCurveRectangleMaskGenerator::toXML(QDomDocument& doc, QDomElement& e) const
{
    KisMaskGenerator::toXML(doc, e);
    e.setAttribute("type", "rect");
    e.setAttribute("softness_curve", curveString());
}

void KisCurveRectangleMaskGenerator::setSoftness(qreal softness)
{
    // performance
    if (!d->dirty && softness == 1.0) return;
    d->dirty = true;
    KisMaskGenerator::setSoftness(softness);
    KisCurveCircleMaskGenerator::transformCurveForSoftness(softness,d->curvePoints, d->curveResolution + 1, d->curveData);
}

