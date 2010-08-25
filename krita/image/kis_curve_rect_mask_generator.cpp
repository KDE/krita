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

struct KisCurveRectangleMaskGenerator::Private {
    QVector<qreal> curveData;
    QString curve;
};

KisCurveRectangleMaskGenerator::KisCurveRectangleMaskGenerator(qreal radius, qreal ratio, qreal fh, qreal fv, int spikes, const KisCubicCurve &curve)
        : KisMaskGenerator(radius, ratio, fh, fv, spikes, RECTANGLE), d(new Private)
{
    d->curve = curve.toString();
    d->curveData = curve.floatTransfer(width() + 1); 
}

KisCurveRectangleMaskGenerator::~KisCurveRectangleMaskGenerator()
{
    delete d;
}

quint8 KisCurveRectangleMaskGenerator::valueAt(qreal x, qreal y) const
{

    if (KisMaskGenerator::d->m_empty) {
        return 255;
    }
    
    double s = qAbs(x) / width();
    double t = qAbs(y) / height();
    
    int sIndex = qRound(s * width());
    int tIndex = qRound(t * width());
    
    int sIndexInverted = width() - sIndex;
    int tIndexInverted = width() - tIndex;
    
    qreal blend = (d->curveData.at(sIndex) * (1.0 - d->curveData.at(sIndexInverted)) *
                  d->curveData.at(tIndex) * (1.0 - d->curveData.at(tIndexInverted)));
    
    return (1.0 - blend) * 255;
}

void KisCurveRectangleMaskGenerator::toXML(QDomDocument& doc, QDomElement& e) const
{
    KisMaskGenerator::toXML(doc, e);
    e.setAttribute("type", "curve_rect");
    e.setAttribute("softness_curve", d->curve);
}

