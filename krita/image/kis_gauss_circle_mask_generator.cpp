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

#include <KoVcMultiArchBuildSupport.h> //MSVC requires that Vc come first
#include <cmath>

#include <QDomDocument>
#include <QVector>
#include <QPointF>

#include <KoColorSpaceConstants.h>

#include "kis_fast_math.h"

#include "kis_base_mask_generator.h"
#include "kis_gauss_circle_mask_generator.h"
#include "kis_antialiasing_fade_maker.h"

#define M_SQRT_2 1.41421356237309504880

#ifdef Q_OS_WIN
// on windows we get our erf() from boost
#include <boost/math/special_functions/erf.hpp>
#define erf(x) boost::math::erf(x)
#endif


struct Q_DECL_HIDDEN KisGaussCircleMaskGenerator::Private
{
    Private(bool enableAntialiasing)
        : fadeMaker(*this, enableAntialiasing)
    {
    }

    qreal ycoef;
    qreal fade;
    qreal center, distfactor, alphafactor;
    KisAntialiasingFadeMaker1D<Private> fadeMaker;

    inline quint8 value(qreal dist) const;
};

KisGaussCircleMaskGenerator::KisGaussCircleMaskGenerator(qreal diameter, qreal ratio, qreal fh, qreal fv, int spikes, bool antialiasEdges)
    : KisMaskGenerator(diameter, ratio, fh, fv, spikes, antialiasEdges, CIRCLE, GaussId), d(new Private(antialiasEdges))
{
    d->ycoef = 1.0 / KisMaskGenerator::d->ratio;
    d->fade = 1.0 - (fh + fv) / 2.0;
    if (d->fade == 0.0) d->fade = 1e-6;
    else if (d->fade == 1.0) d->fade = 1.0 - 1e-6; // would become undefined for fade == 0 or 1
    d->center = (2.5 * (6761.0*d->fade-10000.0))/(M_SQRT_2*6761.0*d->fade);
    d->alphafactor = 255.0 / (2.0 * erf(d->center));
}

void KisGaussCircleMaskGenerator::setScale(qreal scaleX, qreal scaleY)
{
    KisMaskGenerator::setScale(scaleX, scaleY);

    d->distfactor = M_SQRT_2 * 12500.0 / (6761.0 * d->fade * effectiveSrcWidth() / 2.0);
    d->fadeMaker.setRadius(0.5 * effectiveSrcWidth());
}

KisGaussCircleMaskGenerator::~KisGaussCircleMaskGenerator()
{
    delete d;
}

inline quint8 KisGaussCircleMaskGenerator::Private::value(qreal dist) const
{
    dist *= distfactor;
    quint8 ret = alphafactor * (erf(dist + center) - erf(dist - center));
    return (quint8) 255 - ret;
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

    quint8 value;
    if (d->fadeMaker.needFade(dist, &value)) {
        return value;
    }

    return d->value(dist);
}
