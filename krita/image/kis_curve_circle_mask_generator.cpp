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

#include <KoColorSpaceConstants.h>

#include "kis_fast_math.h"

#include "kis_curve_circle_mask_generator.h"
#include "kis_cubic_curve.h"

struct KisCurveCircleMaskGenerator::Private {
    qreal xcoef, ycoef;
    qreal cachedSpikesAngle;
    QVector<qreal> curveData;
    QString curve;
};

KisCurveCircleMaskGenerator::KisCurveCircleMaskGenerator(qreal radius, qreal ratio, qreal fh, qreal fv, int spikes, const KisCubicCurve &curve)
        : KisMaskGenerator(radius, ratio, fh, fv, spikes, CIRCLE), d(new Private)
{
    d->xcoef = 2.0 / width();
    d->ycoef = 2.0 / (KisMaskGenerator::d->m_ratio * width());
    d->cachedSpikesAngle = M_PI / KisMaskGenerator::d->m_spikes;
    d->curveData = curve.floatTransfer( width() + 2);
    d->curve = curve.toString();
}

KisCurveCircleMaskGenerator::~KisCurveCircleMaskGenerator()
{
    delete d;
}

quint8 KisCurveCircleMaskGenerator::valueAt(qreal x, qreal y) const
{
    qreal xr = x;
    qreal yr = qAbs(y);
    if (KisMaskGenerator::d->m_spikes > 2) {
        double angle = (KisFastMath::atan2(yr, xr));

        while (angle > d->cachedSpikesAngle ){
            double sx = xr, sy = yr;

            xr = KisMaskGenerator::d->cs * sx - KisMaskGenerator::d->ss * sy;
            yr = KisMaskGenerator::d->ss * sx + KisMaskGenerator::d->cs * sy;

            angle -= 2 * d->cachedSpikesAngle;
        }
    }

    qreal dist = norme(xr * d->xcoef, yr * d->ycoef);
    if (dist <= 1.0){
        qreal distance = dist * width();
    
        quint16 alphaValue = distance;
        qreal alphaValueF = distance - alphaValue;
           
        qreal alpha = (
            (1.0 - alphaValueF) * d->curveData.at(alphaValue) + 
                    alphaValueF * d->curveData.at(alphaValue+1));
        return (1.0 - alpha) * 255;
    }            
    return 255;
}

void KisCurveCircleMaskGenerator::toXML(QDomDocument& doc, QDomElement& e) const
{
    KisMaskGenerator::toXML(doc, e);
    e.setAttribute("type", "curve_circle");
    e.setAttribute("softness_curve", d->curve);
}

