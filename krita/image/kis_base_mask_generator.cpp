/*
 *  Copyright (c) 2004,2007-2009 Cyrille Berger <cberger@cberger.net>
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

#include "kis_mask_generator.h"

#include <cmath>

#include <QDomDocument>

#include "kis_circle_mask_generator.h"
#include "kis_rect_mask_generator.h"
#include "kis_cubic_curve.h"
#include "kis_curve_circle_mask_generator.h"
#include "kis_curve_rect_mask_generator.h"

KisMaskGenerator::KisMaskGenerator(qreal radius, qreal ratio, qreal fh, qreal fv, int spikes, Type type, const KoID& id) : d(new Private), m_id(id)
{
    d->m_radius = radius;
    d->m_ratio = ratio;
    d->m_fh = 0.5 * fh;
    d->m_fv = 0.5 * fv;
    d->softness = 1.0; // by default don't change fade/softness/hardness
    d->m_spikes = spikes;
    d->type = type;
    init();
}

KisMaskGenerator::~KisMaskGenerator()
{
    delete d;
}

void KisMaskGenerator::init()
{
    d->cs = cos(- 2 * M_PI / d->m_spikes);
    d->ss = sin(- 2 * M_PI / d->m_spikes);
    d->m_empty = (d->m_ratio == 0.0 || d->m_radius == 0.0);
}

void KisMaskGenerator::toXML(QDomDocument& doc, QDomElement& e) const
{
    Q_UNUSED(doc);
    e.setAttribute("radius", d->m_radius);
    e.setAttribute("ratio", d->m_ratio);
    e.setAttribute("hfade", horizontalFade());
    e.setAttribute("vfade", verticalFade());
    e.setAttribute("spikes", d->m_spikes);
    e.setAttribute("id", id());
}

KisMaskGenerator* KisMaskGenerator::fromXML(const QDomElement& elt)
{
    double radius = elt.attribute("radius", "1.0").toDouble();
    double ratio = elt.attribute("ratio", "1.0").toDouble();
    double hfade = elt.attribute("hfade", "0.0").toDouble();
    double vfade = elt.attribute("vfade", "0.0").toDouble();
    int spikes = elt.attribute("spikes", "2").toInt();
    QString typeShape = elt.attribute("type", "circle");
    QString id = elt.attribute("id", DefaultId.id());
    
    if (id == DefaultId.id()){
        if (typeShape == "circle"){
            return new KisCircleMaskGenerator(radius, ratio, hfade, vfade, spikes);
        }else{
            return new KisRectangleMaskGenerator(radius, ratio, hfade, vfade, spikes);
        }
    }
    
    if (id == SoftId.id()){
        KisCubicCurve curve;
        curve.fromString(elt.attribute("softness_curve","0,0;1,1"));

        if (typeShape == "circle"){
            return new KisCurveCircleMaskGenerator(radius, ratio, hfade, vfade, spikes, curve);
        }else{
            return new KisCurveRectangleMaskGenerator(radius, ratio, hfade, vfade, spikes, curve);
        }
    }
    
    // if unknown
    return new KisCircleMaskGenerator(radius, ratio, hfade, vfade, spikes);
}

qreal KisMaskGenerator::width() const
{
    return d->m_radius;
}

qreal KisMaskGenerator::height() const
{
    if (d->m_spikes == 2) {
        return d->m_radius * d->m_ratio;
    }
    return d->m_radius;
}

qreal KisMaskGenerator::radius() const
{
    return d->m_radius;
}

qreal KisMaskGenerator::ratio() const
{
    return d->m_ratio;
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
    return 2.0 * d->m_fh; // 'cause in init we divide it again
}

qreal KisMaskGenerator::verticalFade() const
{
    return 2.0 * d->m_fv; // 'cause in init we divide it again
}

int KisMaskGenerator::spikes() const
{
    return d->m_spikes;
}

KisMaskGenerator::Type KisMaskGenerator::type() const
{
    return d->type;
}

QList< KoID > KisMaskGenerator::maskGeneratorIds()
{
    QList<KoID> ids;
    ids << DefaultId << SoftId;
    return ids;
}
