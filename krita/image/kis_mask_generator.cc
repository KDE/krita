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

KisMaskGenerator::KisMaskGenerator(double width, double height, double fh, double fv) : m_radius(width), m_ratio(height/width), m_fh( 2.0 * fh / width), m_fv( 2.0 * fv / height ), m_spikes(2)
{
}

KisMaskGenerator::KisMaskGenerator(double radius, double ratio, double fh, double fv, int spikes) : m_radius(radius), m_ratio(ratio), m_fh(0.5 * fh), m_fv(0.5 * fv), m_spikes(spikes)
{
}

void KisMaskGenerator::toXML(QDomDocument& , QDomElement& e) const
{
    e.setAttribute("autobrush_radius", m_radius);
    e.setAttribute("autobrush_ratio", m_ratio);
    e.setAttribute("autobrush_hfade", m_fh);
    e.setAttribute("autobrush_vfade", m_fv);
    e.setAttribute("autobrush_spikes", m_spikes);
}

KisMaskGenerator* KisMaskGenerator::fromXML(const QDomElement& elt)
{
    if( elt.hasAttribute("autobrush_radius") )
    {
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

KisCircleMaskGenerator::KisCircleMaskGenerator(double w, double h, double fh, double fv)
        : KisMaskGenerator(w, h, 0.5 * w - fh, 0.5 * h - fv),
        m_xcenter(w / 2.0),
        m_ycenter(h / 2.0),
        m_xcoef(2.0 / w),
        m_ycoef(2.0 / h),
        m_xfadecoef((m_fh == 0) ? 1 : (1.0 / (m_fh*width()))),
        m_yfadecoef((m_fv == 0) ? 1 : (1.0 / (m_fv*height())))
{
}

KisCircleMaskGenerator::KisCircleMaskGenerator(double radius, double ratio, double fh, double fv, int spikes)
        : KisMaskGenerator(radius, ratio, fh, fv, spikes),
        m_xcenter(width() / 2.0),
        m_ycenter(height() / 2.0),
        m_xcoef(2.0 / width()),
        m_ycoef(2.0 / height()),
        m_xfadecoef((m_fh == 0) ? 1 : (1.0 / (m_fh*width()))),
        m_yfadecoef((m_fv == 0) ? 1 : (1.0 / (m_fv*height())))
{
}
quint8 KisCircleMaskGenerator::valueAt(double x, double y)
{
    double xr = (x /*- m_xcenter*/);
    double yr = (y /*- m_ycenter*/);
    double n = norme(xr * m_xcoef, yr * m_ycoef);
    
//     double cs = cos (- 2 * M_PI / m_spikes);
//     double ss = sin (- 2 * M_PI / m_spikes);
//     
//     if( m_spikes > 2 )
//     {
//         double angle = atan2 (yr, xr);
// 
//         while (angle > M_PI / m_spikes)
//         {
//             double sx = xr, sy = yr;
// 
//             xr = cs * sx - ss * sy;
//             yr = ss * sx + cs * sy;
// 
//             angle -= 2 * M_PI / m_spikes;
//         }
//     }

    
    if (n > 1) {
        return 255;
    } else {
        double normeFade = norme(xr * m_xfadecoef, yr * m_yfadecoef);
        if (normeFade > 1) {
            double xle, yle;
            // xle stands for x-coordinate limit exterior
            // yle stands for y-coordinate limit exterior
            // we are computing the coordinate on the external ellipse in order to compute
            // the fade value
            if (xr == 0) {
                xle = 0;
                yle = yr > 0 ? 1 / m_ycoef : -1 / m_ycoef;
            } else {
                double c = yr / (double)xr;
                xle = sqrt(1 / norme(m_xcoef, c * m_ycoef));
                xle = xr > 0 ? xle : -xle;
                yle = xle * c;
            }
            // On the internal limit of the fade area, normeFade is equal to 1
            double normeFadeLimitE = norme(xle * m_xfadecoef, yle * m_yfadecoef);
            return (uchar)(255 * (normeFade - 1) / (normeFadeLimitE - 1));
        } else {
            return 0;
        }
    }
}

void KisCircleMaskGenerator::toXML(QDomDocument& d, QDomElement& e) const
{
    KisMaskGenerator::toXML(d, e);
    e.setAttribute("autobrush_type", "circle");
}

KisRectangleMaskGenerator::KisRectangleMaskGenerator(double w, double h, double fh, double fv)
        : KisMaskGenerator(w, h, 0.5 * w - fh, 0.5 * h - fv),
        m_xcenter(w / 2.0),
        m_ycenter(h / 2.0),
        m_c(fv / fh)
{
}

KisRectangleMaskGenerator::KisRectangleMaskGenerator(double radius, double ratio, double fh, double fv, int spikes)
        : KisMaskGenerator(radius, ratio, fh, fv, spikes),
        m_xcenter(width() / 2.0),
        m_ycenter(height() / 2.0),
        m_c(m_fv / m_fh)
{
}
quint8 KisRectangleMaskGenerator::valueAt(double x, double y)
{
    double xr = qAbs(x /*- m_xcenter*/) / width();
    double yr = qAbs(y /*- m_ycenter*/) / height();
    if (xr > m_fh || yr > m_fv ) {
        if (yr <= ((xr - m_fh ) * m_c + m_fv )) {
            return (uchar)(255 * (xr - 0.5 * m_fh ) / ( 1.0 - 0.5 * m_fh) );
        } else {
            return (uchar)(255 * (yr - 0.5 * m_fv ) /( 1.0 - 0.5 * m_fv) );
        }
    } else {
        return 0;
    }
}

void KisRectangleMaskGenerator::toXML(QDomDocument& d, QDomElement& e) const
{
    KisMaskGenerator::toXML(d, e);
    e.setAttribute("autobrush_type", "rect");
}

