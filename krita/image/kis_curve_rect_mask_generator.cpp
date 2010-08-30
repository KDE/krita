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

#include "kis_curve_rect_mask_generator.h"

#include <cmath>

#include <QDomDocument>
#include <QVector>
#include "kis_cubic_curve.h"
#include <QPointF>

struct KisCurveRectangleMaskGenerator::Private {
    QVector<qreal> curveData;
    QList<QPointF> curvePoints;
    int curveResolution;
    QString curve;
    bool dirty;
};

KisCurveRectangleMaskGenerator::KisCurveRectangleMaskGenerator(qreal diameter, qreal ratio, qreal fh, qreal fv, int spikes, const KisCubicCurve &curve)
        : KisMaskGenerator(diameter, ratio, fh, fv, spikes, RECTANGLE, SoftId), d(new Private)
{
    d->curveResolution = qRound(width());
    d->curveData = curve.floatTransfer( d->curveResolution + 1); 
    d->curvePoints = curve.points();
    d->curve = curve.toString();
    d->dirty = false;
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
    
    double s = qAbs(x) / width();
    double t = qAbs(y) / height();
    
    int sIndex = qRound(s * (d->curveResolution));
    int tIndex = qRound(t * (d->curveResolution));
    
    int sIndexInverted = width() - sIndex;
    int tIndexInverted = width() - tIndex;
    
    qreal blend = (d->curveData.at(sIndex) * (1.0 - d->curveData.at(sIndexInverted)) *
                  d->curveData.at(tIndex) * (1.0 - d->curveData.at(tIndexInverted)));
    
    return (1.0 - blend) * 255;
}

void KisCurveRectangleMaskGenerator::toXML(QDomDocument& doc, QDomElement& e) const
{
    KisMaskGenerator::toXML(doc, e);
    e.setAttribute("type", "rect");
    e.setAttribute("softness_curve", d->curve);
}

void KisCurveRectangleMaskGenerator::setSoftness(qreal softness)
{
    // performance
    if (!d->dirty && softness == 1.0) return;
    d->dirty = true;
    KisMaskGenerator::setSoftness(softness);
    KisCurveCircleMaskGenerator::transformCurveForSoftness(softness,d->curvePoints, d->curveResolution + 1, d->curveData);
}

