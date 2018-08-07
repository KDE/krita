/*
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include <kis_fast_math.h>
#include "kis_antialiasing_fade_maker.h"
#include "kis_brush_mask_applicator_factories.h"
#include "kis_brush_mask_applicator_base.h"

#include "kis_curve_rect_mask_generator.h"
#include "kis_curve_rect_mask_generator_p.h"
#include "kis_cubic_curve.h"


KisCurveRectangleMaskGenerator::KisCurveRectangleMaskGenerator(qreal diameter, qreal ratio, qreal fh, qreal fv, int spikes, const KisCubicCurve &curve, bool antialiasEdges)
    : KisMaskGenerator(diameter, ratio, fh, fv, spikes, antialiasEdges, RECTANGLE, SoftId), d(new Private(antialiasEdges))
{
    d->curveResolution = qRound( qMax(width(),height()) * OVERSAMPLING);
    d->curveData = curve.floatTransfer( d->curveResolution + 1);
    d->curvePoints = curve.points();
    setCurveString(curve.toString());
    d->dirty = false;

    setScale(1.0, 1.0);

    d->applicator.reset(createOptimizedClass<MaskApplicatorFactory<KisCurveRectangleMaskGenerator, KisBrushMaskVectorApplicator> >(this));
}

KisCurveRectangleMaskGenerator::KisCurveRectangleMaskGenerator(const KisCurveRectangleMaskGenerator &rhs)
    : KisMaskGenerator(rhs),
      d(new Private(*rhs.d))
{
    d->applicator.reset(createOptimizedClass<MaskApplicatorFactory<KisCurveRectangleMaskGenerator, KisBrushMaskVectorApplicator> >(this));
}

KisMaskGenerator* KisCurveRectangleMaskGenerator::clone() const
{
    return new KisCurveRectangleMaskGenerator(*this);
}

void KisCurveRectangleMaskGenerator::setScale(qreal scaleX, qreal scaleY)
{
    KisMaskGenerator::setScale(scaleX, scaleY);

    qreal halfWidth = 0.5 * effectiveSrcWidth();
    qreal halfHeight = 0.5 * effectiveSrcHeight();

    d->xcoeff = 1.0 / halfWidth;
    d->ycoeff = 1.0 / halfHeight;

    d->fadeMaker.setLimits(halfWidth, halfHeight);
}

KisCurveRectangleMaskGenerator::~KisCurveRectangleMaskGenerator()
{
}

quint8 KisCurveRectangleMaskGenerator::Private::value(qreal xr, qreal yr) const
{
    xr = qAbs(xr) * xcoeff;
    yr = qAbs(yr) * ycoeff;

    int sIndex = qRound(xr * (curveResolution));
    int tIndex = qRound(yr * (curveResolution));

    int sIndexInverted = curveResolution - sIndex;
    int tIndexInverted = curveResolution - tIndex;

    qreal blend = (curveData.at(sIndex) * (1.0 - curveData.at(sIndexInverted)) *
                   curveData.at(tIndex) * (1.0 - curveData.at(tIndexInverted)));

    return (1.0 - blend) * 255;
}

quint8 KisCurveRectangleMaskGenerator::valueAt(qreal x, qreal y) const
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

void KisCurveRectangleMaskGenerator::toXML(QDomDocument& doc, QDomElement& e) const
{
    KisMaskGenerator::toXML(doc, e);
    e.setAttribute("softness_curve", curveString());
}

void KisCurveRectangleMaskGenerator::setSoftness(qreal softness)
{
    // performance
    if (!d->dirty && softness == 1.0) return;
    d->dirty = true;
    KisMaskGenerator::setSoftness(softness);
    KisCurveCircleMaskGenerator::transformCurveForSoftness(softness,d->curvePoints, d->curveResolution + 1, d->curveData);
    d->dirty = false;
}

bool KisCurveRectangleMaskGenerator::shouldVectorize() const
{
    return !shouldSupersample() && spikes() == 2;
}

KisBrushMaskApplicatorBase* KisCurveRectangleMaskGenerator::applicator()
{
    return d->applicator.data();
}

void KisCurveRectangleMaskGenerator::resetMaskApplicator(bool forceScalar)
{
    d->applicator.reset(createOptimizedClass<MaskApplicatorFactory<KisCurveRectangleMaskGenerator, KisBrushMaskVectorApplicator> >(this,forceScalar));
}

