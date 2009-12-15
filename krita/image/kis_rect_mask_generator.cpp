/*
 *  Copyright (c) 2004,2007-2009 Cyrille Berger <cberger@cberger.net>
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

#include <math.h>

#include <QDomDocument>

struct KisRectangleMaskGenerator::Private {
    double m_c;
};

KisRectangleMaskGenerator::KisRectangleMaskGenerator(double w, double h, double fh, double fv)
        : KisMaskGenerator(w, h, 0.5 * w - fh, 0.5 * h - fv), d(new Private)
{
    if (KisMaskGenerator::d->m_fv == 0 &&
        KisMaskGenerator::d->m_fh == 0) {
        d->m_c = 0;
    }
    else {
        d->m_c = (KisMaskGenerator::d->m_fv / KisMaskGenerator::d->m_fh);
        Q_ASSERT(!isnan(d->m_c));
    }
}

KisRectangleMaskGenerator::KisRectangleMaskGenerator(double radius, double ratio, double fh, double fv, int spikes)
        : KisMaskGenerator(radius, ratio, fh, fv, spikes), d(new Private)
{
    if (KisMaskGenerator::d->m_fv == 0 &&
        KisMaskGenerator::d->m_fh == 0) {
        d->m_c = 0;
    }
    else {
        d->m_c = (KisMaskGenerator::d->m_fv / KisMaskGenerator::d->m_fh);
        Q_ASSERT(!isnan(d->m_c));
    }
}

KisRectangleMaskGenerator::~KisRectangleMaskGenerator()
{
    delete d;
}

quint8 KisRectangleMaskGenerator::valueAt(double x, double y) const
{

    if (KisMaskGenerator::d->m_empty) return 255;
    double xr = qAbs(x /*- m_xcenter*/) / width();
    double yr = qAbs(y /*- m_ycenter*/) / height();
    if (xr > KisMaskGenerator::d->m_fh || yr > KisMaskGenerator::d->m_fv) {
        if (yr <= ((xr - KisMaskGenerator::d->m_fh) * d->m_c + KisMaskGenerator::d->m_fv)) {
            return (uchar)(255 *(xr - 0.5 * KisMaskGenerator::d->m_fh) / (1.0 - 0.5 * KisMaskGenerator::d->m_fh));
        } else {
            return (uchar)(255 *(yr - 0.5 * KisMaskGenerator::d->m_fv) / (1.0 - 0.5 * KisMaskGenerator::d->m_fv));
        }
    } else {
        return 0;
    }
}

void KisRectangleMaskGenerator::toXML(QDomDocument& d, QDomElement& e) const
{
    KisMaskGenerator::toXML(d, e);
    e.setAttribute("autobrush_type", "rect");
}

