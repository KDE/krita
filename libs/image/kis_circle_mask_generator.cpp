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
#if defined _MSC_VER
// Lets shut up the "possible loss of data" and "forcing value to bool 'true' or 'false'
#pragma warning ( push )
#pragma warning ( disable : 4244 )
#pragma warning ( disable : 4800 )
#endif
#include <Vc/Vc>
#include <Vc/IO>
#if defined _MSC_VER
#pragma warning ( pop )
#endif
#endif

#include <QDomDocument>

#include "kis_fast_math.h"
#include "kis_circle_mask_generator.h"
#include "kis_circle_mask_generator_p.h"
#include "kis_base_mask_generator.h"
#include "kis_brush_mask_applicator_factories.h"
#include "kis_brush_mask_applicator_base.h"


KisCircleMaskGenerator::KisCircleMaskGenerator(qreal diameter, qreal ratio, qreal fh, qreal fv, int spikes, bool antialiasEdges)
    : KisMaskGenerator(diameter, ratio, fh, fv, spikes, antialiasEdges, CIRCLE, DefaultId),
      d(new Private)
{
    setScale(1.0, 1.0);

    // store the variable locally to allow vector implementation read it easily
    d->copyOfAntialiasEdges = antialiasEdges;

    d->applicator.reset(createOptimizedClass<MaskApplicatorFactory<KisCircleMaskGenerator, KisBrushMaskVectorApplicator> >(this));
}

KisCircleMaskGenerator::KisCircleMaskGenerator(const KisCircleMaskGenerator &rhs)
    : KisMaskGenerator(rhs),
      d(new Private(*rhs.d))
{
    d->applicator.reset(createOptimizedClass<MaskApplicatorFactory<KisCircleMaskGenerator, KisBrushMaskVectorApplicator> >(this));
}

KisMaskGenerator* KisCircleMaskGenerator::clone() const
{
    return new KisCircleMaskGenerator(*this);
}

void KisCircleMaskGenerator::setScale(qreal scaleX, qreal scaleY)
{
    KisMaskGenerator::setScale(scaleX, scaleY);

    d->xcoef = 2.0 / effectiveSrcWidth();
    d->ycoef = 2.0 / effectiveSrcHeight();
    d->xfadecoef = (horizontalFade() == 0) ? 1 : (2.0 / (horizontalFade() * effectiveSrcWidth()));
    d->yfadecoef = (verticalFade() == 0)   ? 1 : (2.0 / (verticalFade() * effectiveSrcHeight()));
    d->transformedFadeX = KisMaskGenerator::softness() * d->xfadecoef;
    d->transformedFadeY = KisMaskGenerator::softness() * d->yfadecoef;

    d->noFading = !d->copyOfAntialiasEdges &&
        qFuzzyCompare(d->xcoef, d->transformedFadeX) &&
        qFuzzyCompare(d->ycoef, d->transformedFadeY);
}

KisCircleMaskGenerator::~KisCircleMaskGenerator()
{
}

bool KisCircleMaskGenerator::shouldSupersample() const
{
    return effectiveSrcWidth() < 10 || effectiveSrcHeight() < 10;
}

bool KisCircleMaskGenerator::shouldVectorize() const
{
    return !shouldSupersample() && spikes() == 2;
}

KisBrushMaskApplicatorBase* KisCircleMaskGenerator::applicator()
{
    return d->applicator.data();
}

quint8 KisCircleMaskGenerator::valueAt(qreal x, qreal y) const
{
    if (isEmpty()) return 255;
    qreal xr = (x /*- m_xcenter*/);
    qreal yr = qAbs(y /*- m_ycenter*/);
    fixRotation(xr, yr);

    qreal n = norme(xr * d->xcoef, yr * d->ycoef);
    if (n > 1.0) return 255;

    // we add +1.0 to ensure correct antialiasing on the border
    if (antialiasEdges()) {
        xr = qAbs(xr) + 1.0;
        yr = qAbs(yr) + 1.0;
    }

    qreal nf = norme(xr * d->transformedFadeX,
                     yr * d->transformedFadeY);

    if (nf < 1.0) return 0;
    return 255 * n * (nf - 1.0) / (nf - n);
}

void KisCircleMaskGenerator::setSoftness(qreal softness)
{
    KisMaskGenerator::setSoftness(softness);
    qreal safeSoftnessCoeff = qreal(1.0) / qMax(qreal(0.01), softness);

    d->transformedFadeX = d->xfadecoef * safeSoftnessCoeff;
    d->transformedFadeY = d->yfadecoef * safeSoftnessCoeff;
}

void KisCircleMaskGenerator::resetMaskApplicator(bool forceScalar)
{
    d->applicator.reset(createOptimizedClass<MaskApplicatorFactory<KisCircleMaskGenerator, KisBrushMaskVectorApplicator> >(this,forceScalar));
}
