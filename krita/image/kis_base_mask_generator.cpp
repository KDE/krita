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

KisMaskGenerator::KisMaskGenerator(double width, double height, double fh, double fv) : d(new Private)
{
    d->m_radius = width;
    d->m_ratio = height / width;
    d->m_fh = 2.0 * fh / width;
    d->m_fv = 2.0 * fv / height;
    d->m_spikes = 2;
    init();
}

KisMaskGenerator::KisMaskGenerator(double radius, double ratio, double fh, double fv, int spikes) : d(new Private)
{
    d->m_radius = radius;
    d->m_ratio = ratio;
    d->m_fh = 0.5 * fh;
    d->m_fv = 0.5 * fv;
    d->m_spikes = spikes;
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

    e.setAttribute("autobrush_radius", d->m_radius);
    e.setAttribute("autobrush_ratio", d->m_ratio);
    e.setAttribute("autobrush_hfade", d->m_fh * 2); // 'cause in init we divide it again
    e.setAttribute("autobrush_vfade", d->m_fv * 2); // 'cause in init we divide it again
    e.setAttribute("autobrush_spikes", d->m_spikes);

}

KisMaskGenerator* KisMaskGenerator::fromXML(const QDomElement& elt)
{
    if (elt.hasAttribute("autobrush_radius")) {
        double radius = elt.attribute("autobrush_radius", "1.0").toDouble();
        double ratio = elt.attribute("autobrush_ratio", "1.0").toDouble();
        double hfade = elt.attribute("autobrush_hfade", "0.0").toDouble();
        double vfade = elt.attribute("autobrush_vfade", "0.0").toDouble();
        int spikes = elt.attribute("autobrush_spikes", "2").toInt();
        QString typeShape = elt.attribute("autobrush_type", "circle");

        if (typeShape == "circle") {
            return new KisCircleMaskGenerator(radius, ratio, hfade, vfade, spikes);
        } else {
            return new KisRectangleMaskGenerator(radius, ratio, hfade, vfade, spikes);
        }
    } else {
        double width = elt.attribute("autobrush_width", "1.0").toDouble();
        double height = elt.attribute("autobrush_height", "1.0").toDouble();
        double hfade = elt.attribute("autobrush_hfade", "1.0").toDouble();
        double vfade = elt.attribute("autobrush_vfade", "1.0").toDouble();
        QString typeShape = elt.attribute("autobrush_type", "circle");

        if (typeShape == "circle") {
            return new KisCircleMaskGenerator(width, height, hfade, vfade);
        } else {
            return new KisRectangleMaskGenerator(width, height, hfade, vfade);
        }
    }
}

quint8 KisMaskGenerator::interpolatedValueAt(double x, double y)
{
    double x_i = floor(x);
    double x_f = x - x_i;
    if (x_f < 0.0) {
        x_f *= -1.0;
    }
    double x_f_r = 1.0 - x_f;
    double y_i = floor(y);
    double y_f = fabs(y - y_i);
    if (y_f < 0.0) {
        y_f *= -1.0;
    }
    double y_f_r = 1.0 - y_f;
    return (x_f_r * y_f_r * valueAt(x_i , y_i) +
            x_f   * y_f_r * valueAt(x_i + 1, y_i) +
            x_f_r * y_f   * valueAt(x_i,  y_i + 1) +
            x_f   * y_f   * valueAt(x_i + 1,  y_i + 1));
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

