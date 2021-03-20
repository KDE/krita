/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "kis_curve_circle_mask_generator.h"
#include "kis_curve_circle_mask_generator_p.h"
#include "kis_cubic_curve.h"


KisCurveCircleMaskGenerator::KisCurveCircleMaskGenerator(qreal diameter, qreal ratio, qreal fh, qreal fv, int spikes, const KisCubicCurve &curve, bool antialiasEdges)
    : KisMaskGenerator(diameter, ratio, fh, fv, spikes, antialiasEdges, CIRCLE, SoftId), d(new Private(antialiasEdges))
{
    // here we set resolution for the maximum size of the brush!
    d->curveResolution = qRound(qMax(width(), height()) * OVERSAMPLING);
    d->curveData = curve.floatTransfer(d->curveResolution + 2);
    d->curvePoints = curve.points();
    setCurveString(curve.toString());
    d->dirty = false;

    setScale(1.0, 1.0);

    d->applicator.reset(createOptimizedClass<MaskApplicatorFactory<KisCurveCircleMaskGenerator, KisBrushMaskVectorApplicator> >(this));
}

KisCurveCircleMaskGenerator::KisCurveCircleMaskGenerator(const KisCurveCircleMaskGenerator &rhs)
    : KisMaskGenerator(rhs),
      d(new Private(*rhs.d))
{
    d->applicator.reset(createOptimizedClass<MaskApplicatorFactory<KisCurveCircleMaskGenerator, KisBrushMaskVectorApplicator> >(this));
}

KisCurveCircleMaskGenerator::~KisCurveCircleMaskGenerator()
{
}

KisMaskGenerator* KisCurveCircleMaskGenerator::clone() const
{
    return new KisCurveCircleMaskGenerator(*this);
}

void KisCurveCircleMaskGenerator::setScale(qreal scaleX, qreal scaleY)
{
    KisMaskGenerator::setScale(scaleX, scaleY);

    qreal width = effectiveSrcWidth();
    qreal height = effectiveSrcHeight();

    d->xcoef = 2.0 / width;
    d->ycoef = 2.0 / height;

    d->fadeMaker.setSquareNormCoeffs(d->xcoef, d->ycoef);
}

bool KisCurveCircleMaskGenerator::shouldVectorize() const
{
    return !shouldSupersample() && spikes() == 2;
}

KisBrushMaskApplicatorBase* KisCurveCircleMaskGenerator::applicator()
{
    return d->applicator.data();
}

inline quint8 KisCurveCircleMaskGenerator::Private::value(qreal dist) const
{
    qreal distance = dist * curveResolution;

    quint16 alphaValue = distance;
    qreal alphaValueF = distance - alphaValue;

    qreal alpha = (
        (1.0 - alphaValueF) * curveData.at(alphaValue) +
        alphaValueF * curveData.at(alphaValue+1));

    return (1.0 - alpha) * 255;
}

quint8 KisCurveCircleMaskGenerator::valueAt(qreal x, qreal y) const
{
    if (isEmpty()) return 255;
    qreal xr = x;
    qreal yr = qAbs(y);
    fixRotation(xr, yr);

    qreal dist = norme(xr * d->xcoef, yr * d->ycoef);

    quint8 value;
    if (d->fadeMaker.needFade(dist, &value)) {
        return value;
    }

    return d->value(dist);
}

void KisCurveCircleMaskGenerator::toXML(QDomDocument& doc, QDomElement& e) const
{
    KisMaskGenerator::toXML(doc, e);
    e.setAttribute("softness_curve", curveString());
}

void KisCurveCircleMaskGenerator::setSoftness(qreal softness)
{
    // performance
    if (!d->dirty && softness == 1.0) return;

    d->dirty = true;
    KisMaskGenerator::setSoftness(softness);
    KisCurveCircleMaskGenerator::transformCurveForSoftness(softness,d->curvePoints, d->curveResolution+2, d->curveData);
    d->dirty = false;
}

void KisCurveCircleMaskGenerator::transformCurveForSoftness(qreal softness,const QList<QPointF> &points, int curveResolution, QVector< qreal >& result)
{
    QList<QPointF> newList = points;
    newList.detach();

    int size = newList.size();
    if (size == 2){
        // make place for new point in the centre
        newList.append(newList.at(1));
        newList[1] = (newList.at(0) + newList.at(2)) * 0.5;
        // transoform it
        newList[1].setY(qBound<qreal>(0.0,newList.at(1).y() * softness,1.0));
    }else{
        // transform all points except first and last
        for (int i = 1; i < size-1; i++){
            newList[i].setY(qBound<qreal>(0.0,newList.at(i).y() * softness,1.0));
        }
    }

    // compute the data
    KisCubicCurve curve(newList);
    result = curve.floatTransfer( curveResolution );
}

void KisCurveCircleMaskGenerator::resetMaskApplicator(bool forceScalar)
{
    d->applicator.reset(createOptimizedClass<MaskApplicatorFactory<KisCurveCircleMaskGenerator, KisBrushMaskVectorApplicator> >(this,forceScalar));
}
