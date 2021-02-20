/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  SPDX-FileCopyrightText: 2011 Geoffry Song <goffrie@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <compositeops/KoVcMultiArchBuildSupport.h> //MSVC requires that Vc come first
#include <cmath>
#include <algorithm>

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
#include <QVector>
#include <QPointF>

#include <KoColorSpaceConstants.h>

#include "kis_fast_math.h"

#include "kis_base_mask_generator.h"
#include "kis_antialiasing_fade_maker.h"
#include "kis_brush_mask_applicator_factories.h"
#include "kis_brush_mask_applicator_base.h"
#include "kis_gauss_rect_mask_generator.h"
#include "kis_gauss_rect_mask_generator_p.h"

#define M_SQRT_2 1.41421356237309504880

#ifdef Q_OS_WIN
// on windows we get our erf() from boost
#include <boost/math/special_functions/erf.hpp>
#define erf(x) boost::math::erf(x)
#endif



KisGaussRectangleMaskGenerator::KisGaussRectangleMaskGenerator(qreal diameter, qreal ratio, qreal fh, qreal fv, int spikes, bool antialiasEdges)
    : KisMaskGenerator(diameter, ratio, fh, fv, spikes, antialiasEdges, RECTANGLE, GaussId), d(new Private(antialiasEdges))
{
    setScale(1.0, 1.0);

    d->applicator.reset(createOptimizedClass<MaskApplicatorFactory<KisGaussRectangleMaskGenerator, KisBrushMaskVectorApplicator> >(this));
}

KisGaussRectangleMaskGenerator::KisGaussRectangleMaskGenerator(const KisGaussRectangleMaskGenerator &rhs)
    : KisMaskGenerator(rhs),
      d(new Private(*rhs.d))
{
    d->applicator.reset(createOptimizedClass<MaskApplicatorFactory<KisGaussRectangleMaskGenerator, KisBrushMaskVectorApplicator> >(this));
}

KisMaskGenerator* KisGaussRectangleMaskGenerator::clone() const
{
    return new KisGaussRectangleMaskGenerator(*this);
}

void KisGaussRectangleMaskGenerator::setScale(qreal scaleX, qreal scaleY)
{
    KisMaskGenerator::setScale(scaleX, scaleY);

    qreal width = effectiveSrcWidth();
    qreal height = effectiveSrcHeight();

    qreal xfade = (1.0 - horizontalFade()/2.0) * width * 0.1;
    qreal yfade = (1.0 - verticalFade()/2.0) * height * 0.1;
    d->xfade = 1.0 / (M_SQRT_2 * xfade);
    d->yfade = 1.0 / (M_SQRT_2 * yfade);
    d->halfWidth = width * 0.5 - 2.5 * xfade;
    d->halfHeight = height * 0.5 - 2.5 * yfade;
    d->alphafactor = 255.0 / (4.0 * erf(d->halfWidth * d->xfade) * erf(d->halfHeight * d->yfade));

    if (std::isnan(d->alphafactor)) d->alphafactor = 0.0f; // erf can return nan if ratio is 0

    d->fadeMaker.setLimits(0.5 * width, 0.5 * height);
}

KisGaussRectangleMaskGenerator::~KisGaussRectangleMaskGenerator()
{
}

inline quint8 KisGaussRectangleMaskGenerator::Private::value(qreal xr, qreal yr) const
{
    return (quint8) 255 - (quint8) (alphafactor * (erf((halfWidth + xr) * xfade) + erf((halfWidth - xr) * xfade))
                                    * (erf((halfHeight + yr) * yfade) + erf((halfHeight - yr) * yfade)));
}

quint8 KisGaussRectangleMaskGenerator::valueAt(qreal x, qreal y) const
{
    if (isEmpty()) return 255;
    qreal xr = x;
    qreal yr = qAbs(y);
    fixRotation(xr, yr);

    quint8 value;
    if (d->fadeMaker.needFade(xr, yr, &value)) {
        return value;
    }

    return d->value(xr, yr);
}

bool KisGaussRectangleMaskGenerator::shouldVectorize() const
{
    return !shouldSupersample() && spikes() == 2;
}

KisBrushMaskApplicatorBase* KisGaussRectangleMaskGenerator::applicator()
{
    return d->applicator.data();
}

void KisGaussRectangleMaskGenerator::resetMaskApplicator(bool forceScalar)
{
    d->applicator.reset(createOptimizedClass<MaskApplicatorFactory<KisGaussRectangleMaskGenerator, KisBrushMaskVectorApplicator> >(this,forceScalar));
}
