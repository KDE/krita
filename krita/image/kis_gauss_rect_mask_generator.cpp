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

#define M_SQRT_2 1.41421356237309504880

struct KisGaussRectangleMaskGenerator::Private {
    qreal xfade, yfade;
    qreal halfWidth, halfHeight;
    qreal alphafactor;
};

KisGaussRectangleMaskGenerator::KisGaussRectangleMaskGenerator(qreal diameter, qreal ratio, qreal fh, qreal fv, int spikes)
        : KisMaskGenerator(diameter, ratio, fh, fv, spikes, RECTANGLE, GaussId), d(new Private)
{
    qreal xfade = (1.0 - fh) * width() * 0.1;
    qreal yfade = (1.0 - fv) * height() * 0.1;
    d->xfade = 1.0 / (M_SQRT_2 * xfade);
    d->yfade = 1.0 / (M_SQRT_2 * yfade);
    d->halfWidth = width() * 0.5 - 2.5 * xfade;
    d->halfHeight = height() * 0.5 - 2.5 * yfade;
    d->alphafactor = 255.0 / (4.0 * erf(d->halfWidth * d->xfade) * erf(d->halfHeight * d->yfade));
}

KisGaussRectangleMaskGenerator::~KisGaussRectangleMaskGenerator()
{
    delete d;
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
    return (quint8) 255 - (quint8) (d->alphafactor * (erf((d->halfWidth + xr) * d->xfade) + erf((d->halfWidth - xr) * d->xfade))
                                                  * (erf((d->halfHeight + yr) * d->yfade) + erf((d->halfHeight - yr) * d->yfade)));
}

void KisGaussRectangleMaskGenerator::toXML(QDomDocument& doc, QDomElement& e) const
{
    KisMaskGenerator::toXML(doc, e);
    e.setAttribute("type", "rect");
}
