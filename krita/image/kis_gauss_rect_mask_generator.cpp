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
#include <algorithm>

#include <QDomDocument>
#include <QVector>
#include <QPointF>

#include <KoColorSpaceConstants.h>

#include "kis_fast_math.h"

#include "kis_base_mask_generator.h"
#include "kis_gauss_rect_mask_generator.h"
#include "kis_antialiasing_fade_maker.h"

#define M_SQRT_2 1.41421356237309504880

#ifdef Q_OS_WIN
// on windows we get our erf() from boost
#include <boost/math/special_functions/erf.hpp>
#define erf(x) boost::math::erf(x)
#endif

struct KisGaussRectangleMaskGenerator::Private
{
    Private(bool enableAntialiasing)
        : fadeMaker(*this, enableAntialiasing)
    {
    }

    qreal xfade, yfade;
    qreal halfWidth, halfHeight;
    qreal alphafactor;

    KisAntialiasingFadeMaker2D <Private> fadeMaker;

    inline quint8 value(qreal x, qreal y) const;
};

KisGaussRectangleMaskGenerator::KisGaussRectangleMaskGenerator(qreal diameter, qreal ratio, qreal fh, qreal fv, int spikes, bool antialiasEdges)
    : KisMaskGenerator(diameter, ratio, fh, fv, spikes, antialiasEdges, RECTANGLE, GaussId), d(new Private(antialiasEdges))
{
    setScale(1.0, 1.0);
}

void KisGaussRectangleMaskGenerator::setScale(qreal scaleX, qreal scaleY)
{
    KisMaskGenerator::setScale(scaleX, scaleY);

    qreal width = effectiveSrcWidth();
    qreal height = effectiveSrcHeight();

    qreal xfade = (1.0 - KisMaskGenerator::d->fh) * width * 0.1;
    qreal yfade = (1.0 - KisMaskGenerator::d->fv) * height * 0.1;
    d->xfade = 1.0 / (M_SQRT_2 * xfade);
    d->yfade = 1.0 / (M_SQRT_2 * yfade);
    d->halfWidth = width * 0.5 - 2.5 * xfade;
    d->halfHeight = height * 0.5 - 2.5 * yfade;
    d->alphafactor = 255.0 / (4.0 * erf(d->halfWidth * d->xfade) * erf(d->halfHeight * d->yfade));

    d->fadeMaker.setLimits(0.5 * width, 0.5 * height);
}

KisGaussRectangleMaskGenerator::~KisGaussRectangleMaskGenerator()
{
    delete d;
}

inline quint8 KisGaussRectangleMaskGenerator::Private::value(qreal xr, qreal yr) const
{
    return (quint8) 255 - (quint8) (alphafactor * (erf((halfWidth + xr) * xfade) + erf((halfWidth - xr) * xfade))
                                    * (erf((halfHeight + yr) * yfade) + erf((halfHeight - yr) * yfade)));
}

quint8 KisGaussRectangleMaskGenerator::valueAt(qreal x, qreal y) const
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

    quint8 value;
    if (d->fadeMaker.needFade(xr, yr, &value)) {
        return value;
    }

    return d->value(xr, yr);
}
