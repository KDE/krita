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

#include <KoColorSpaceConstants.h>

#include "kis_fast_math.h"

#include "kis_base_mask_generator.h"
#include "kis_curve_circle_mask_generator.h"
#include "kis_cubic_curve.h"
#include "kis_antialiasing_fade_maker.h"



struct KisCurveCircleMaskGenerator::Private
{
    Private(bool enableAntialiasing)
        : fadeMaker(*this, enableAntialiasing)
    {
    }

    qreal xcoef, ycoef;
    qreal curveResolution;
    QVector<qreal> curveData;
    QList<QPointF> curvePoints;
    bool dirty;

    KisAntialiasingFadeMaker1D<Private> fadeMaker;
    inline quint8 value(qreal dist) const;
};

KisCurveCircleMaskGenerator::KisCurveCircleMaskGenerator(qreal diameter, qreal ratio, qreal fh, qreal fv, int spikes, const KisCubicCurve &curve, bool antialiasEdges)
    : KisMaskGenerator(diameter, ratio, fh, fv, spikes, antialiasEdges, CIRCLE, SoftId), d(new Private(antialiasEdges))
{
    // here we set resolution for the maximum size of the brush!
    d->curveResolution = qRound(qMax(width(), height()) * OVERSAMPLING);
    d->curveData = curve.floatTransfer(d->curveResolution + 2);
    d->curvePoints = curve.points();
    setCurveString(curve.toString());
    d->dirty = false;

    setScale(1.0, 1.0);
}

void KisCurveCircleMaskGenerator::setScale(qreal scaleX, qreal scaleY)
{
    KisMaskGenerator::setScale(scaleX, scaleY);

    qreal width = effectiveSrcWidth();
    qreal height = effectiveSrcHeight();

    d->xcoef = 2.0 / width;
    d->ycoef = 2.0 / height;

    d->fadeMaker.setSquareNormCoeffs(d->xcoef, d->ycoef);
}

KisCurveCircleMaskGenerator::~KisCurveCircleMaskGenerator()
{
    delete d;
}

bool KisCurveCircleMaskGenerator::shouldSupersample() const
{
    return effectiveSrcWidth() < 10 || effectiveSrcHeight() < 10;
}

inline quint8 KisCurveCircleMaskGenerator::Private::value(qreal dist) const
{
    qreal distance = dist * curveResolution;

    quint16 alphaValue = distance;
    qreal alphaValueF = distance - alphaValue;

    qreal alpha = (
        (1.0 - alphaValueF) * curveData.at(alphaValue) +
        alphaValueF * curveData.at(alphaValue+1));
    return (1.0 - alpha) * 255;
}

quint8 KisCurveCircleMaskGenerator::valueAt(qreal x, qreal y) const
{
    qreal xr = x;
    qreal yr = qAbs(y);
    if (KisMaskGenerator::d->spikes > 2) {
        double angle = (KisFastMath::atan2(yr, xr));

        while (angle > KisMaskGenerator::d->cachedSpikesAngle ){
            double sx = xr, sy = yr;

            xr = KisMaskGenerator::d->cs * sx - KisMaskGenerator::d->ss * sy;
            yr = KisMaskGenerator::d->ss * sx + KisMaskGenerator::d->cs * sy;

            angle -= 2 * KisMaskGenerator::d->cachedSpikesAngle;
        }
    }

    qreal dist = norme(xr * d->xcoef, yr * d->ycoef);

    quint8 value;
    if (d->fadeMaker.needFade(dist, &value)) {
        return value;
    }

    return d->value(dist);
}

void KisCurveCircleMaskGenerator::toXML(QDomDocument& doc, QDomElement& e) const
{
    KisMaskGenerator::toXML(doc, e);
    e.setAttribute("softness_curve", curveString());
}

void KisCurveCircleMaskGenerator::setSoftness(qreal softness)
{
    // performance
    if (!d->dirty && softness == 1.0) return;

    d->dirty = true;
    KisMaskGenerator::setSoftness(softness);
    KisCurveCircleMaskGenerator::transformCurveForSoftness(softness,d->curvePoints, d->curveResolution+2, d->curveData);
    d->dirty = false;
}

void KisCurveCircleMaskGenerator::transformCurveForSoftness(qreal softness,const QList<QPointF> &points, int curveResolution, QVector< qreal >& result)
{
    QList<QPointF> newList = points;
    newList.detach();

    int size = newList.size();
    if (size == 2){
        // make place for new point in the centre
        newList.append(newList.at(1));
        newList[1] = (newList.at(0) + newList.at(2)) * 0.5;
        // transoform it
        newList[1].setY(qBound<qreal>(0.0,newList.at(1).y() * softness,1.0));
    }else{
        // transform all points except first and last
        for (int i = 1; i < size-1; i++){
            newList[i].setY(qBound<qreal>(0.0,newList.at(i).y() * softness,1.0));
        }
    }

    // compute the data
    KisCubicCurve curve(newList);
    result = curve.floatTransfer( curveResolution );
}
