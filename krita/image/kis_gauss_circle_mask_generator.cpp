/*
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  Copyright (c) 2011 Geoffry Song <goffrie@gmail.com>
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
#include "kis_gauss_circle_mask_generator.h"

#define M_SQRT_2 1.41421356237309504880

#ifdef Q_OS_WIN
// on windows we get our erf() from boost
#include <boost/math/special_functions/erf.hpp>
#define erf(x) boost::math::erf(x)
#endif


struct KisGaussCircleMaskGenerator::Private {
    qreal ycoef;
    qreal center, distfactor, alphafactor;
};

KisGaussCircleMaskGenerator::KisGaussCircleMaskGenerator(qreal diameter, qreal ratio, qreal fh, qreal fv, int spikes)
        : KisMaskGenerator(diameter, ratio, fh, fv, spikes, CIRCLE, GaussId), d(new Private)
{
    d->ycoef = 1.0 / KisMaskGenerator::d->ratio;
    qreal fade = 1.0 - (fh + fv) / 2.0;
    if (fade == 0.0) fade = 1e-6;
    else if (fade == 1.0) fade = 1.0 - 1e-6; // would become undefined for fade == 0 or 1
    d->center = (2.5 * (6761.0*fade-10000.0))/(M_SQRT_2*6761.0*fade);
    d->alphafactor = 255.0 / (2.0 * erf(d->center));
    d->distfactor = M_SQRT_2 * 12500.0 / (6761.0 * fade * diameter / 2.0);
}

KisGaussCircleMaskGenerator::~KisGaussCircleMaskGenerator()
{
    delete d;
}

quint8 KisGaussCircleMaskGenerator::valueAt(qreal x, qreal y) const
{
    qreal xr = x;
    qreal yr = qAbs(y);
    if (KisMaskGenerator::d->spikes > 2) {
        double angle = KisFastMath::atan2(yr, xr);

        while (angle > KisMaskGenerator::d->cachedSpikesAngle) {
            double sx = xr, sy = yr;

            xr = KisMaskGenerator::d->cs * sx - KisMaskGenerator::d->ss * sy;
            yr = KisMaskGenerator::d->ss * sx + KisMaskGenerator::d->cs * sy;

            angle -= 2 * KisMaskGenerator::d->cachedSpikesAngle;
        }
    }

    qreal dist = sqrt(norme(xr, yr * d->ycoef));
    dist *= d->distfactor;
    quint8 ret = d->alphafactor * (erf(dist + d->center) - erf(dist - d->center));
    return (quint8) 255 - ret;
}

void KisGaussCircleMaskGenerator::toXML(QDomDocument& doc, QDomElement& e) const
{
    KisMaskGenerator::toXML(doc, e);
    e.setAttribute("type", "circle");
}
