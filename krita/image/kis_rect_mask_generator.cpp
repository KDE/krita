/*
 *  Copyright (c) 2004,2007,2008,2009.2010 Cyrille Berger <cberger@cberger.net>
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

#include "kis_rect_mask_generator.h"

#include <cmath>

#include <QDomDocument>

struct KisRectangleMaskGenerator::Private {
    double m_c;
    double m_halfWidth, m_halfHeight;
};

KisRectangleMaskGenerator::KisRectangleMaskGenerator(qreal radius, qreal ratio, qreal fh, qreal fv, int spikes)
        : KisMaskGenerator(radius, ratio, fh, fv, spikes, RECTANGLE, DefaultId), d(new Private)
{
    if (KisMaskGenerator::d->fv == 0 &&
            KisMaskGenerator::d->fh == 0) {
        d->m_c = 0;
    } else {
        d->m_c = (KisMaskGenerator::d->fv / KisMaskGenerator::d->fh);
        Q_ASSERT(!isnan(d->m_c));
    }
    d->m_halfWidth = KisMaskGenerator::d->diameter * 0.5;
    d->m_halfHeight = d->m_halfWidth * KisMaskGenerator::d->ratio;
}

KisRectangleMaskGenerator::~KisRectangleMaskGenerator()
{
    delete d;
}

quint8 KisRectangleMaskGenerator::valueAt(qreal x, qreal y) const
{
    if (KisMaskGenerator::d->empty) return 255;
    
    if(x > d->m_halfWidth || x < -d->m_halfWidth || y > d->m_halfHeight || y < -d->m_halfHeight) return 255;
    
    double xr = qAbs(x /*- m_xcenter*/) / width();
    double yr = qAbs(y /*- m_ycenter*/) / height();
    
    qreal fhTransformed = KisMaskGenerator::d->fh * softness();
    qreal fvTransformed = KisMaskGenerator::d->fv * softness();

    if( xr > fhTransformed )
    {
        if( yr > xr )
        {
            return (uchar)(255 *(yr - fvTransformed) / (0.5 - fvTransformed));
        } else {
            return (uchar)(255 *(xr - fhTransformed) / (0.5 - fhTransformed));
        }
    } else if( yr > fvTransformed )
    {
        return (uchar)(255 *(yr - fvTransformed) / (0.5 - fvTransformed));
    } else if(xr < fhTransformed && yr < fvTransformed )
    {
        return 0;
    } else {
        return 255;
    }
}

void KisRectangleMaskGenerator::toXML(QDomDocument& d, QDomElement& e) const
{
    KisMaskGenerator::toXML(d, e);
    e.setAttribute("type", "rect");
}

