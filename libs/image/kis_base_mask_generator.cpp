/*
 *  SPDX-FileCopyrightText: 2004, 2007-2009 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  SPDX-FileCopyrightText: 2011 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <compositeops/KoVcMultiArchBuildSupport.h> //vc.h must come first

#include "kis_brush_mask_applicator_factories.h"
#include "kis_mask_generator.h"
#include "kis_brush_mask_applicator_base.h"

#include <cmath>
#include "kis_fast_math.h"

#include <QDomDocument>

#include "kis_circle_mask_generator.h"
#include "kis_rect_mask_generator.h"
#include "kis_gauss_circle_mask_generator.h"
#include "kis_gauss_rect_mask_generator.h"
#include "kis_cubic_curve.h"
#include "kis_curve_circle_mask_generator.h"
#include "kis_curve_rect_mask_generator.h"
#include <kis_dom_utils.h>

struct KisMaskGenerator::Private {
    Private()
        : diameter(1.0),
          ratio(1.0),
          softness(1.0),
          fh(1.0),
          fv(1.0),
          cs(0.0),
          ss(0.0),
          cachedSpikesAngle(0.0),
          spikes(2),
          empty(true),
          antialiasEdges(false),
          type(CIRCLE),
          scaleX(1.0),
          scaleY(1.0)
    {
    }

    Private(const Private &rhs)
        : diameter(rhs.diameter),
          ratio(rhs.ratio),
          softness(rhs.softness),
          fh(rhs.fh),
          fv(rhs.fv),
          cs(rhs.cs),
          ss(rhs.ss),
          cachedSpikesAngle(rhs.cachedSpikesAngle),
          spikes(rhs.spikes),
          empty(rhs.empty),
          antialiasEdges(rhs.antialiasEdges),
          type(rhs.type),
          curveString(rhs.curveString),
          scaleX(rhs.scaleX),
          scaleY(rhs.scaleY)
    {
    }

    qreal diameter, ratio;
    qreal softness;
    qreal fh, fv;
    qreal cs, ss;
    qreal cachedSpikesAngle;
    int spikes;
    bool empty;
    bool antialiasEdges;
    Type type;
    QString curveString;
    qreal scaleX;
    qreal scaleY;
    QScopedPointer<KisBrushMaskApplicatorBase> defaultMaskProcessor;
};


KisMaskGenerator::KisMaskGenerator(qreal diameter, qreal ratio, qreal fh, qreal fv, int spikes, bool antialiasEdges, Type type, const KoID& id)
    : d(new Private), m_id(id)
{
    d->diameter = diameter;
    d->ratio = ratio;
    d->fh = 0.5 * fh;
    d->fv = 0.5 * fv;
    d->softness = 1.0; // by default don't change fade/softness/hardness
    d->spikes = spikes;
    d->cachedSpikesAngle = M_PI / d->spikes;
    d->type = type;
    d->antialiasEdges = antialiasEdges;
    d->scaleX = 1.0;
    d->scaleY = 1.0;
    init();
}

KisMaskGenerator::~KisMaskGenerator()
{
}

KisMaskGenerator::KisMaskGenerator(const KisMaskGenerator &rhs)
    : d(new Private(*rhs.d)),
      m_id(rhs.m_id)
{
}

void KisMaskGenerator::init()
{
    d->cs = cos(- 2 * M_PI / d->spikes);
    d->ss = sin(- 2 * M_PI / d->spikes);
    d->empty = (d->ratio == 0.0 || d->diameter == 0.0);
}

bool KisMaskGenerator::shouldSupersample() const
{
    return false;
}

bool KisMaskGenerator::shouldVectorize() const
{
    return false;
}


bool KisMaskGenerator::isEmpty() const
{
    return d->empty;
}

KisBrushMaskApplicatorBase* KisMaskGenerator::applicator()
{
    if (!d->defaultMaskProcessor) {
        d->defaultMaskProcessor.reset(
            createOptimizedClass<MaskApplicatorFactory<KisMaskGenerator, KisBrushMaskScalarApplicator> >(this));
    }

    return d->defaultMaskProcessor.data();
}

void KisMaskGenerator::toXML(QDomDocument& doc, QDomElement& e) const
{
    Q_UNUSED(doc);
    //e.setAttribute("radius", d->radius);
    e.setAttribute("diameter", QString::number(d->diameter));
    e.setAttribute("ratio", QString::number(d->ratio));
    e.setAttribute("hfade", QString::number(horizontalFade()));
    e.setAttribute("vfade", QString::number(verticalFade()));
    e.setAttribute("spikes", d->spikes);
    e.setAttribute("type", d->type == CIRCLE ? "circle" : "rect");
    e.setAttribute("antialiasEdges", d->antialiasEdges);
    e.setAttribute("id", id());
}

KisMaskGenerator* KisMaskGenerator::fromXML(const QDomElement& elt)
{
    double diameter = 1.0;
    // backward compatibility -- it was mistakenly named radius for 2.2
    if (elt.hasAttribute("radius")){
        diameter = KisDomUtils::toDouble(elt.attribute("radius", "1.0"));
    }
    else /*if (elt.hasAttribute("diameter"))*/{
        diameter = KisDomUtils::toDouble(elt.attribute("diameter", "1.0"));
    }
    double ratio = KisDomUtils::toDouble(elt.attribute("ratio", "1.0"));
    double hfade = KisDomUtils::toDouble(elt.attribute("hfade", "0.0"));
    double vfade = KisDomUtils::toDouble(elt.attribute("vfade", "0.0"));

    int spikes = elt.attribute("spikes", "2").toInt();
    QString typeShape = elt.attribute("type", "circle");
    QString id = elt.attribute("id", DefaultId.id());
    bool antialiasEdges = elt.attribute("antialiasEdges", "0").toInt();

    if (id == DefaultId.id()) {
        if (typeShape == "circle") {
            return new KisCircleMaskGenerator(diameter, ratio, hfade, vfade, spikes, antialiasEdges);
        } else {
            return new KisRectangleMaskGenerator(diameter, ratio, hfade, vfade, spikes, antialiasEdges);
        }
    }

    if (id == SoftId.id()) {
        KisCubicCurve curve;
        curve.fromString(elt.attribute("softness_curve","0,0;1,1"));

        if (typeShape == "circle") {
            return new KisCurveCircleMaskGenerator(diameter, ratio, hfade, vfade, spikes, curve, antialiasEdges);
        } else {
            return new KisCurveRectangleMaskGenerator(diameter, ratio, hfade, vfade, spikes, curve, antialiasEdges);
        }
    }

    if (id == GaussId.id()) {
        if (typeShape == "circle") {
            return new KisGaussCircleMaskGenerator(diameter, ratio, hfade, vfade, spikes, antialiasEdges);
        } else {
            return new KisGaussRectangleMaskGenerator(diameter, ratio, hfade, vfade, spikes, antialiasEdges);
        }
    }

    // if unknown
    return new KisCircleMaskGenerator(diameter, ratio, hfade, vfade, spikes, true);
}

qreal KisMaskGenerator::width() const
{
    return d->diameter;
}

qreal KisMaskGenerator::height() const
{
    if (d->spikes == 2) {
        return d->diameter * d->ratio;
    }
    return d->diameter;
}

qreal KisMaskGenerator::effectiveSrcWidth() const
{
    return d->diameter * d->scaleX;
}

qreal KisMaskGenerator::effectiveSrcHeight() const
{
    /**
     * This height is related to the source of the brush mask, so we
     * don't take spikes into account, they will be generated from
     * this data.
     */
    return d->diameter * d->ratio * d->scaleY;
}

qreal KisMaskGenerator::diameter() const
{
    return d->diameter;
}

void KisMaskGenerator::setDiameter(qreal value)
{
    d->diameter = value;
    init();
    setScale(d->scaleX, d->scaleY);
}

qreal KisMaskGenerator::ratio() const
{
    return d->ratio;
}

qreal KisMaskGenerator::softness() const
{
    return d->softness;
}


void KisMaskGenerator::setSoftness(qreal softness)
{
    d->softness = softness;
}


qreal KisMaskGenerator::horizontalFade() const
{
    return 2.0 * d->fh; // 'cause in init we divide it again
}

qreal KisMaskGenerator::verticalFade() const
{
    return 2.0 * d->fv; // 'cause in init we divide it again
}

int KisMaskGenerator::spikes() const
{
    return d->spikes;
}

KisMaskGenerator::Type KisMaskGenerator::type() const
{
    return d->type;
}

QList< KoID > KisMaskGenerator::maskGeneratorIds()
{
    QList<KoID> ids;
    ids << DefaultId << SoftId << GaussId;
    return ids;
}

QString KisMaskGenerator::curveString() const
{
    return d->curveString;
}

void KisMaskGenerator::setCurveString(const QString& curveString)
{
    d->curveString = curveString;
}

bool KisMaskGenerator::antialiasEdges() const
{
    return d->antialiasEdges;
}

void KisMaskGenerator::setScale(qreal scaleX, qreal scaleY)
{
    d->scaleX = scaleX;
    d->scaleY = scaleY;
}

void KisMaskGenerator::fixRotation(qreal &xr, qreal &yr) const
{
    if (d->spikes > 2) {
        double angle = (KisFastMath::atan2(yr, xr));

        while (angle > d->cachedSpikesAngle){
            double sx = xr;
            double sy = yr;

            xr = d->cs * sx - d->ss * sy;
            yr = d->ss * sx + d->cs * sy;

            angle -= 2 * d->cachedSpikesAngle;
        }
    }
}
