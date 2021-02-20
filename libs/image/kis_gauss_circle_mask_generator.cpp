/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  SPDX-FileCopyrightText: 2011 Geoffry Song <goffrie@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <compositeops/KoVcMultiArchBuildSupport.h> //MSVC requires that Vc come first
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
#include <QVector>
#include <QPointF>

#include <KoColorSpaceConstants.h>

#include "kis_fast_math.h"

#include "kis_base_mask_generator.h"
#include "kis_antialiasing_fade_maker.h"
#include "kis_brush_mask_applicator_factories.h"
#include "kis_brush_mask_applicator_base.h"
#include "kis_gauss_circle_mask_generator.h"
#include "kis_gauss_circle_mask_generator_p.h"

#define M_SQRT_2 1.41421356237309504880

#ifdef Q_OS_WIN
// on windows we get our erf() from boost
#include <boost/math/special_functions/erf.hpp>
#define erf(x) boost::math::erf(x)
#endif


KisGaussCircleMaskGenerator::KisGaussCircleMaskGenerator(qreal diameter, qreal ratio, qreal fh, qreal fv, int spikes, bool antialiasEdges)
    : KisMaskGenerator(diameter, ratio, fh, fv, spikes, antialiasEdges, CIRCLE, GaussId),
      d(new Private(antialiasEdges))
{
    d->ycoef = 1.0 / ratio;
    d->fade = 1.0 - (fh + fv) / 2.0;

    if (d->fade == 0.0) d->fade = 1e-6;
    else if (d->fade == 1.0) d->fade = 1.0 - 1e-6; // would become undefined for fade == 0 or 1

    d->center = (2.5 * (6761.0*d->fade-10000.0))/(M_SQRT_2*6761.0*d->fade);
    d->alphafactor = 255.0 / (2.0 * erf(d->center));

    d->applicator.reset(createOptimizedClass<MaskApplicatorFactory<KisGaussCircleMaskGenerator, KisBrushMaskVectorApplicator> >(this));

}

KisGaussCircleMaskGenerator::KisGaussCircleMaskGenerator(const KisGaussCircleMaskGenerator &rhs)
    : KisMaskGenerator(rhs),
      d(new Private(*rhs.d))
{
    d->applicator.reset(createOptimizedClass<MaskApplicatorFactory<KisGaussCircleMaskGenerator, KisBrushMaskVectorApplicator> >(this));
}

KisMaskGenerator* KisGaussCircleMaskGenerator::clone() const
{
    return new KisGaussCircleMaskGenerator(*this);
}

void KisGaussCircleMaskGenerator::setScale(qreal scaleX, qreal scaleY)
{
    KisMaskGenerator::setScale(scaleX, scaleY);
    d->ycoef = scaleX / (scaleY * ratio());

    d->distfactor = M_SQRT_2 * 12500.0 / (6761.0 * d->fade * effectiveSrcWidth() / 2.0);
    d->fadeMaker.setRadius(0.5 * effectiveSrcWidth());
}

KisGaussCircleMaskGenerator::~KisGaussCircleMaskGenerator()
{
}

inline quint8 KisGaussCircleMaskGenerator::Private::value(qreal dist) const
{
    dist *= distfactor;
    quint8 ret = alphafactor * (erf(dist + center) - erf(dist - center));
    return (quint8) 255 - ret;
}

bool KisGaussCircleMaskGenerator::shouldVectorize() const
{
    return !shouldSupersample() && spikes() == 2;
}

KisBrushMaskApplicatorBase* KisGaussCircleMaskGenerator::applicator()
{
    return d->applicator.data();
}

quint8 KisGaussCircleMaskGenerator::valueAt(qreal x, qreal y) const
{
    if (isEmpty()) return 255;
    qreal xr = x;
    qreal yr = qAbs(y);
    fixRotation(xr, yr);

    qreal dist = sqrt(norme(xr, yr * d->ycoef));

    quint8 value;
    if (d->fadeMaker.needFade(dist, &value)) {
        return value;
    }

    return d->value(dist);
}

void KisGaussCircleMaskGenerator::resetMaskApplicator(bool forceScalar)
{
    d->applicator.reset(createOptimizedClass<MaskApplicatorFactory<KisGaussCircleMaskGenerator, KisBrushMaskVectorApplicator> >(this,forceScalar));
}
