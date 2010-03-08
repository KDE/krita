/*
 *  Copyright (c) 2004,2007-2009 Cyrille Berger <cberger@cberger.net>
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

#include <math.h>

#include <QDomDocument>

#include "kis_circle_mask_generator.h"
#include "kis_rect_mask_generator.h"

KisMaskGenerator::KisMaskGenerator(double width, double height, double fh, double fv, Type type) : d(new Private)
{
    d->m_radius = width;
    d->m_ratio = height / width;
    d->m_fh = 2.0 * fh / width;
    d->m_fv = 2.0 * fv / height;
    d->m_spikes = 2;
    d->type = type;
    init();
}

KisMaskGenerator::KisMaskGenerator(double radius, double ratio, double fh, double fv, int spikes, Type type) : d(new Private)
{
    d->m_radius = radius;
    d->m_ratio = ratio;
    d->m_fh = 0.5 * fh;
    d->m_fv = 0.5 * fv;
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
    e.setAttribute("hfade", d->m_fh * 2); // 'cause in init we divide it again
    e.setAttribute("vfade", d->m_fv * 2); // 'cause in init we divide it again
    e.setAttribute("spikes", d->m_spikes);

}

KisMaskGenerator* KisMaskGenerator::fromXML(const QDomElement& elt)
{
    double radius = elt.attribute("radius", "1.0").toDouble();
    double ratio = elt.attribute("ratio", "1.0").toDouble();
    double hfade = elt.attribute("hfade", "0.0").toDouble();
    double vfade = elt.attribute("vfade", "0.0").toDouble();
    int spikes = elt.attribute("spikes", "2").toInt();
    QString typeShape = elt.attribute("type", "circle");

    if (typeShape == "circle") {
        return new KisCircleMaskGenerator(radius, ratio, hfade, vfade, spikes);
    } else {
        return new KisRectangleMaskGenerator(radius, ratio, hfade, vfade, spikes);
    }
}

double KisMaskGenerator::width() const
{
    return d->m_radius;
}

double KisMaskGenerator::height() const
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

qreal KisMaskGenerator::horizontalFade() const
{
    return d->m_fh;
}

qreal KisMaskGenerator::verticalFade() const
{
    return d->m_fv;
}

int KisMaskGenerator::spikes() const
{
    return d->m_spikes;
}

KisMaskGenerator::Type KisMaskGenerator::type() const
{
    return d->type;
}
