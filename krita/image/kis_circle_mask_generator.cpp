/*
 *  Copyright (c) 2004,2007-2009 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  Copyright (c) 2012 Sven Langkamp <sven.langkamp@gmail.com>
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

#include <config-vc.h>
#ifdef HAVE_VC
#if defined(__clang__)
#pragma GCC diagnostic ignored "-Wundef"
#pragma GCC diagnostic ignored "-Wlocal-type-template-args"
#endif
#include <Vc/Vc>
#include <Vc/IO>
#endif

#include <QDomDocument>

#include "kis_fast_math.h"
#include "kis_circle_mask_generator.h"
#include "kis_circle_mask_generator_p.h"
#include "kis_base_mask_generator.h"
#include "kis_brush_mask_applicator_factories.h"


KisCircleMaskGenerator::KisCircleMaskGenerator(qreal diameter, qreal ratio, qreal fh, qreal fv, int spikes)
        : KisMaskGenerator(diameter, ratio, fh, fv, spikes, CIRCLE, DefaultId), d(new Private)
{
    d->xcoef = 2.0 / width();
    d->ycoef = 2.0 / (KisMaskGenerator::d->ratio * width());
    d->xfadecoef = (KisMaskGenerator::d->fh == 0) ? 1 : (1.0 / (KisMaskGenerator::d->fh * width()));
    d->yfadecoef = (KisMaskGenerator::d->fv == 0) ? 1 : (1.0 / (KisMaskGenerator::d->fv * KisMaskGenerator::d->ratio * width()));
    d->transformedFadeX = d->xfadecoef * softness();
    d->transformedFadeY = d->yfadecoef * softness();

    d->applicator = createOptimizedClass<MaskApplicatorFactory<KisCircleMaskGenerator, KisBrushMaskVectorApplicator> >(this);
}

KisCircleMaskGenerator::~KisCircleMaskGenerator()
{
    delete d->applicator;
    delete d;
}

bool KisCircleMaskGenerator::shouldSupersample() const
{
    return width() < 10 || KisMaskGenerator::d->ratio * width() < 10;
}

bool KisCircleMaskGenerator::shouldVectorize() const
{
    return !shouldSupersample() && spikes() == 2;
}

KisBrushMaskApplicatorBase* KisCircleMaskGenerator::applicator()
{
    return d->applicator;
}

quint8 KisCircleMaskGenerator::valueAt(qreal x, qreal y) const
{
    if (KisMaskGenerator::d->empty) return 255;
    double xr = (x /*- m_xcenter*/);
    double yr = fabs(y /*- m_ycenter*/);

    if (KisMaskGenerator::d->spikes > 2) {
        double angle = (KisFastMath::atan2(yr, xr));

        while (angle > KisMaskGenerator::d->cachedSpikesAngle ){
            double sx = xr;
            double sy = yr;

            xr = KisMaskGenerator::d->cs * sx - KisMaskGenerator::d->ss * sy;
            yr = KisMaskGenerator::d->ss * sx + KisMaskGenerator::d->cs * sy;

            angle -= 2 * KisMaskGenerator::d->cachedSpikesAngle;
        }
    }

    qreal n = norme(xr * d->xcoef, yr * d->ycoef);
    if (n > 1.0) return 255;

    // we add +1.0 to ensure correct antialising on the border
    qreal nf = norme((qAbs(xr) + 1.0) * d->transformedFadeX,
                     (qAbs(yr) + 1.0) * d->transformedFadeY);

    if (nf < 1.0) return 0;
    return 255 * n * (nf - 1.0) / (nf - n);
}

void KisCircleMaskGenerator::toXML(QDomDocument& d, QDomElement& e) const
{
    KisMaskGenerator::toXML(d, e);
    e.setAttribute("type", "circle");
}

void KisCircleMaskGenerator::setSoftness(qreal softness)
{
    if (softness == 0){
        KisMaskGenerator::setSoftness(1.0);
    }else{
        KisMaskGenerator::setSoftness(1.0 / softness);
    }
    d->transformedFadeX = d->xfadecoef * this->softness();
    d->transformedFadeY = d->yfadecoef * this->softness();
}
