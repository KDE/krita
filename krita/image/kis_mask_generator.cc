/*
 *  Copyright (c) 2004,2007-2008 Cyrille Berger <cberger@cberger.net>
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

void KisMaskGenerator::toXML(QDomDocument& , QDomElement& e) const
{
    e.setAttribute( "autobrush_width", m_w );
    e.setAttribute( "autobrush_height", m_h );
    e.setAttribute( "autobrush_hfade", m_w / 2.0 - m_fh );
    e.setAttribute( "autobrush_vfade", m_h / 2.0 - m_fv );
}

quint8 KisMaskGenerator::interpolatedValueAt(double x, double y)
{
    double x_i = floor(x);
    double x_f = x - x_i;
    if( x_f < 0.0) { x_f *= -1.0; }
    double x_f_r = 1.0 - x_f;
    double y_i = floor(y);
    double y_f = fabs( y - y_i );
    if( y_f < 0.0) { y_f *= -1.0; }
    double y_f_r = 1.0 - y_f;
    return ( x_f_r * y_f_r * valueAt( x_i , y_i  ) + 
                x_f   * y_f_r * valueAt( x_i + 1, y_i ) +
                x_f_r * y_f   * valueAt( x_i,  y_i + 1) +
                x_f   * y_f   * valueAt( x_i + 1,  y_i + 1 ) );
}


KisCircleMaskGenerator::KisCircleMaskGenerator(double w, double h, double fh, double fv)
    : KisMaskGenerator( w, h, w / 2.0 - fh, h / 2.0 - fv),
        m_xcenter ( w / 2.0 ),
        m_ycenter ( h / 2.0 ),
        m_xcoef ( 2.0 / w ),
        m_ycoef ( 2.0 / h ),
        m_xfadecoef ( (m_fh == 0) ? 1 : ( 1.0 / m_fh)),
        m_yfadecoef ( (m_fv == 0) ? 1 : ( 1.0 / m_fv))
{
}
quint8 KisCircleMaskGenerator::valueAt(double x, double y)
{
    double xr = (x /*- m_xcenter*/);
    double yr = (y /*- m_ycenter*/);
    double n = norme( xr * m_xcoef, yr * m_ycoef);
    if( n > 1 )
    {
        return 255;
    }
    else
    {
        double normeFade = norme( xr * m_xfadecoef, yr * m_yfadecoef );
        if( normeFade > 1)
        {
            double xle, yle;
            // xle stands for x-coordinate limit exterior
            // yle stands for y-coordinate limit exterior
            // we are computing the coordinate on the external ellipse in order to compute
            // the fade value
            if( xr == 0 )
            {
                xle = 0;
                yle = yr > 0 ? 1/m_ycoef : -1/m_ycoef;
            } else {
                double c = yr / (double)xr;
                xle = sqrt(1 / norme( m_xcoef, c * m_ycoef ));
                xle = xr > 0 ? xle : -xle;
                yle = xle * c;
            }
            // On the internal limit of the fade area, normeFade is equal to 1
            double normeFadeLimitE = norme( xle * m_xfadecoef, yle * m_yfadecoef );
            return (uchar)(255 * ( normeFade - 1 ) / ( normeFadeLimitE - 1 ));
        } else {
            return 0;
        }
    }
}

void KisCircleMaskGenerator::toXML(QDomDocument& d, QDomElement& e) const
{
    KisMaskGenerator::toXML(d,e);
    e.setAttribute( "autobrush_type", "circle" );
}


KisRectangleMaskGenerator::KisRectangleMaskGenerator(double w, double h, double fh, double fv)
    : KisMaskGenerator( w, h, w / 2.0 - fh, h / 2.0 - fv),
        m_xcenter ( w / 2.0 ),
        m_ycenter ( h / 2.0 ),
        m_c( fv/fh)
{
}
quint8 KisRectangleMaskGenerator::valueAt(double x, double y)
{
    double xr = QABS(x /*- m_xcenter*/);
    double yr = QABS(y /*- m_ycenter*/);
    if( xr > m_fh || yr > m_fv )
    {
        if( yr <= ((xr - m_fh) * m_c + m_fv )  )
        {
            return (uchar)(255 * (xr - m_fh) / (m_w - m_fh));
        } else {
            return (uchar)(255 * (yr - m_fv) / (m_w - m_fv));
        }
    }
    else {
        return 0;
    }
}

void KisRectangleMaskGenerator::toXML(QDomDocument& d, QDomElement& e) const
{
    KisMaskGenerator::toXML(d,e);
    e.setAttribute( "autobrush_type", "rect" );
}

