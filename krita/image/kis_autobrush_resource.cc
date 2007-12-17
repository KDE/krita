/*
 *  Copyright (c) 2004,2007 Cyrille Berger <cberger@cberger.net>
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
 
#include "kis_autobrush_resource.h"
#include <kdebug.h>
#include <math.h>

#include <KoColor.h>

#include "kis_datamanager.h"
#include "kis_paint_device.h"

QImage KisAutobrushShape::createBrush()
{
    QImage img((int)(m_w+0.5), (int)(m_h+0.5), QImage::Format_ARGB32);
    double centerX = img.width() * 0.5;
    double centerY = img.height() * 0.5;
    for(int j = 0; j < m_h; j++)
    {
        for(int i = 0; i < m_w; i++)
        {
            qint8 v = valueAt( i - centerX, j - centerY);
            img.setPixel( i, j, qRgb(v,v,v));
        }
    }
    return img;
}

KisAutobrushCircleShape::KisAutobrushCircleShape(double w, double h, double fh, double fv)
    : KisAutobrushShape( w, h, w / 2.0 - fh, h / 2.0 - fv),
        m_xcenter ( w / 2.0 ),
        m_ycenter ( h / 2.0 ),
        m_xcoef ( 2.0 / w ),
        m_ycoef ( 2.0 / h ),
        m_xfadecoef ( (m_fh == 0) ? 1 : ( 1.0 / m_fh)),
        m_yfadecoef ( (m_fv == 0) ? 1 : ( 1.0 / m_fv))
{
}
quint8 KisAutobrushCircleShape::valueAt(double x, double y)
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

KisAutobrushRectShape::KisAutobrushRectShape(double w, double h, double fh, double fv)
    : KisAutobrushShape( w, h, w / 2.0 - fh, h / 2.0 - fv),
        m_xcenter ( w / 2.0 ),
        m_ycenter ( h / 2.0 ),
        m_c( fv/fh)
{
}
quint8 KisAutobrushRectShape::valueAt(double x, double y)
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

struct KisAutobrushResource::Private {
    KisAutobrushShape* shape;
};

KisAutobrushResource::KisAutobrushResource(KisAutobrushShape* as) : KisBrush(""), d(new Private)
{
    d->shape = as;
    QImage img = d->shape->createBrush();
    setImage(img);
    setBrushType(MASK);
}

KisAutobrushResource::~KisAutobrushResource()
{
    delete d->shape;
    delete d;
}

void KisAutobrushResource::mask(KisPaintDeviceSP dst, double scaleX, double scaleY, double angle, const KisPaintInformation& info, double subPixelX , double subPixelY ) const
{
    Q_UNUSED(info);

        // Generate the paint device from the mask
    const KoColorSpace* cs = dst->colorSpace();
    
    int dstWidth = maskWidth(scaleX, angle);
    int dstHeight = maskHeight(scaleY, angle);
    
    double invScaleX = 1.0 / scaleX;
    double invScaleY = 1.0 / scaleY;
    
    double centerX = dstWidth  * 0.5 - 1.0 + subPixelX;
    double centerY = dstHeight * 0.5 - 1.0 + subPixelY;
    double cosa = cos(angle);
    double sina = sin(angle);
    // Apply the alpha mask
    KisHLineIteratorPixel hiter = dst->createHLineIterator(0, 0, dstWidth);
    for (int y = 0; y < dstHeight; y++)
    {
        while(! hiter.isDone())
        {
            double x_ = ( hiter.x() - centerX) * invScaleX;
            double y_ = ( hiter.y() - centerY) * invScaleY ;
            double x = cosa * x_ - sina * y_;
            double y = sina * x_ + cosa * y_;
            double x_i = floor(x);
            double x_f = x - x_i;
            if( x_f < 0.0) { x_f *= -1.0; }
            double x_f_r = 1.0 - x_f;
            double y_i = floor(y);
            double y_f = fabs( y - y_i );
            if( y_f < 0.0) { y_f *= -1.0; }
            double y_f_r = 1.0 - y_f;
            cs->setAlpha( hiter.rawData(),
                          OPACITY_OPAQUE - (
                                x_f_r * y_f_r * d->shape->valueAt( x_i , y_i  ) + 
                                x_f   * y_f_r * d->shape->valueAt( x_i + 1, y_i ) +
                                x_f_r * y_f   * d->shape->valueAt( x_i,  y_i + 1) +
                                x_f   * y_f   * d->shape->valueAt( x_i + 1,  y_i + 1 ) )
                                  , 1 );
            ++hiter;
        }
        hiter.nextRow();
    }


}
