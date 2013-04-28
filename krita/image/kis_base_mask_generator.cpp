/*
 *  Copyright (c) 2004,2007-2009 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  Copyright (c) 2011 Sven Langkamp <sven.langkamp@gmail.com>
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

#include "kis_mask_generator.h"

#include <cmath>

#include <QDomDocument>

#include "kis_circle_mask_generator.h"
#include "kis_rect_mask_generator.h"
#include "kis_gauss_circle_mask_generator.h"
#include "kis_gauss_rect_mask_generator.h"
#include "kis_cubic_curve.h"
#include "kis_curve_circle_mask_generator.h"
#include "kis_curve_rect_mask_generator.h"
#include "kis_brush_mask_applicator_factories.h"


KisMaskGenerator::KisMaskGenerator(qreal diameter, qreal ratio, qreal fh, qreal fv, int spikes, Type type, const KoID& id) : d(new Private), m_id(id)
{
    d->diameter = diameter;
    d->ratio = ratio;
    d->fh = 0.5 * fh;
    d->fv = 0.5 * fv;
    d->softness = 1.0; // by default don't change fade/softness/hardness
    d->spikes = spikes;
    d->cachedSpikesAngle = M_PI / d->spikes;
    d->type = type;
    d->defaultMaskProcessor = 0;
    init();
}

KisMaskGenerator::~KisMaskGenerator()
{
    delete d->defaultMaskProcessor;
    delete d;
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

KisBrushMaskApplicatorBase* KisMaskGenerator::applicator()
{
    if (!d->defaultMaskProcessor) {
        d->defaultMaskProcessor =
            createOptimizedClass<MaskApplicatorFactory<KisMaskGenerator, KisBrushMaskScalarApplicator> >(this);
    }

    return d->defaultMaskProcessor;
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
    e.setAttribute("id", id());
}

KisMaskGenerator* KisMaskGenerator::fromXML(const QDomElement& elt)
{
    QLocale c(QLocale::German);
    bool result;

    double diameter = 1.0;
    // backward compatibility -- it was mistakenly named radius for 2.2
    if (elt.hasAttribute("radius")){
        diameter = elt.attribute("radius", "1.0").toDouble(&result);
        if (!result) {
            diameter = c.toDouble(elt.attribute("radius"));
        }
    }else /*if (elt.hasAttribute("diameter"))*/{
        diameter = elt.attribute("diameter", "1.0").toDouble(&result);
        if (!result) {
            diameter = c.toDouble(elt.attribute("diameter"));
        }
    }
    double ratio = elt.attribute("ratio", "1.0").toDouble(&result);
    if (!result) {
        ratio = c.toDouble(elt.attribute("ratio"));
    }
    double hfade = elt.attribute("hfade", "0.0").toDouble(&result);
    if (!result) {
        hfade = c.toDouble(elt.attribute("hfade"));
    }
    double vfade = elt.attribute("vfade", "0.0").toDouble(&result);
    if (!result) {
        vfade = c.toDouble(elt.attribute("vfade"));
    }

    int spikes = elt.attribute("spikes", "2").toInt();
    QString typeShape = elt.attribute("type", "circle");
    QString id = elt.attribute("id", DefaultId.id());

    if (id == DefaultId.id()) {
        if (typeShape == "circle") {
            return new KisCircleMaskGenerator(diameter, ratio, hfade, vfade, spikes);
        } else {
            return new KisRectangleMaskGenerator(diameter, ratio, hfade, vfade, spikes);
        }
    }

    if (id == SoftId.id()) {
        KisCubicCurve curve;
        curve.fromString(elt.attribute("softness_curve","0,0;1,1"));

        if (typeShape == "circle") {
            return new KisCurveCircleMaskGenerator(diameter, ratio, hfade, vfade, spikes, curve);
        } else {
            return new KisCurveRectangleMaskGenerator(diameter, ratio, hfade, vfade, spikes, curve);
        }
    }

    if (id == GaussId.id()) {
        if (typeShape == "circle") {
            return new KisGaussCircleMaskGenerator(diameter, ratio, hfade, vfade, spikes);
        } else {
            return new KisGaussRectangleMaskGenerator(diameter, ratio, hfade, vfade, spikes);
        }
    }

    // if unknown
    return new KisCircleMaskGenerator(diameter, ratio, hfade, vfade, spikes);
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

qreal KisMaskGenerator::diameter() const
{
    return d->diameter;
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
