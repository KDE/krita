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

#include "kis_circle_mask_generator.h"

#include <math.h>
#include <QDomDocument>

struct KisCircleMaskGenerator::Private {
    double m_xcoef, m_ycoef;
    double m_xfadecoef, m_yfadecoef;
};

KisCircleMaskGenerator::KisCircleMaskGenerator(double w, double h, double fh, double fv)
        : KisMaskGenerator(w, h, 0.5 * w - fh, 0.5 * h - fv, CIRCLE), d(new Private)
{
    d->m_xcoef = 2.0 / w;
    d->m_ycoef = 2.0 / h;
    d->m_xfadecoef = (KisMaskGenerator::d->m_fh == 0) ? 1 : (1.0 / (KisMaskGenerator::d->m_fh * width()));
    d->m_yfadecoef = (KisMaskGenerator::d->m_fv == 0) ? 1 : (1.0 / (KisMaskGenerator::d->m_fv * height()));
}

KisCircleMaskGenerator::KisCircleMaskGenerator(double radius, double ratio, double fh, double fv, int spikes)
        : KisMaskGenerator(radius, ratio, fh, fv, spikes, CIRCLE), d(new Private)
{
    d->m_xcoef = 2.0 / width();
    d->m_ycoef = 2.0 / (KisMaskGenerator::d->m_ratio * width());
    d->m_xfadecoef = (KisMaskGenerator::d->m_fh == 0) ? 1 : (1.0 / (KisMaskGenerator::d->m_fh * width()));
    d->m_yfadecoef = (KisMaskGenerator::d->m_fv == 0) ? 1 : (1.0 / (KisMaskGenerator::d->m_fv * KisMaskGenerator::d->m_ratio * width()));
}

KisCircleMaskGenerator::~KisCircleMaskGenerator()
{
    delete d;
}

quint8 KisCircleMaskGenerator::valueAt(double x, double y) const
{
    if (KisMaskGenerator::d->m_empty) return 255;
    double xr = (x /*- m_xcenter*/);
    double yr = fabs(y /*- m_ycenter*/);

    if (KisMaskGenerator::d->m_spikes > 2) {
        double angle = (atan2(yr, xr));

        while (angle > M_PI / KisMaskGenerator::d->m_spikes) {
            double sx = xr, sy = yr;

            xr = KisMaskGenerator::d->cs * sx - KisMaskGenerator::d->ss * sy;
            yr = KisMaskGenerator::d->ss * sx + KisMaskGenerator::d->cs * sy;

            angle -= 2 * M_PI / KisMaskGenerator::d->m_spikes;
        }
    }

    double n = norme(xr * d->m_xcoef, yr * d->m_ycoef);

    if (n > 1) {
        return 255;
    } else {
        double normeFade = norme(xr * d->m_xfadecoef, yr * d->m_yfadecoef);
        if (normeFade > 1) {
            double xle, yle;
            // xle stands for x-coordinate limit exterior
            // yle stands for y-coordinate limit exterior
            // we are computing the coordinate on the external ellipse in order to compute
            // the fade value
            if (xr == 0) {
                xle = 0;
                yle = yr > 0 ? 1 / d->m_ycoef : -1 / d->m_ycoef;
            } else {
                double c = yr / (double)xr;
                xle = sqrt(1 / norme(d->m_xcoef, c * d->m_ycoef));
                xle = xr > 0 ? xle : -xle;
                yle = xle * c;
            }
            // On the internal limit of the fade area, normeFade is equal to 1
            double normeFadeLimitE = norme(xle * d->m_xfadecoef, yle * d->m_yfadecoef);
            return (uchar)(255 *(normeFade - 1) / (normeFadeLimitE - 1));
        } else {
            return 0;
        }
    }
}

void KisCircleMaskGenerator::toXML(QDomDocument& d, QDomElement& e) const
{
    KisMaskGenerator::toXML(d, e);
    e.setAttribute("autobrush_type", "circle");
}

