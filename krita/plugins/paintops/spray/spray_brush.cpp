/*
 *  Copyright (c) 2008 Lukas Tvrdy <lukast.dev@gmail.com>
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

#include "spray_brush.h"

#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorTransformation.h>

#include <QVariant>
#include <QHash>

#include "kis_random_accessor.h"
#include <cmath>

#ifdef _WIN32
#define srand48 srand
#define drand48 rand
#endif

SprayBrush::SprayBrush(const KoColor &inkColor)
{
    m_inkColor = inkColor;
    m_counter = 0;
}

SprayBrush::SprayBrush()
{
    m_radius = 0;
    m_counter = 0;
}



void SprayBrush::paint(KisPaintDeviceSP dev, qreal x, qreal y, const KoColor &color)
{
    srand(time(0));
    // jitter radius
    int tmpRadius = m_radius;
    if (m_jitterSize){
        m_radius = m_radius * drand48();
    }

    // jitter movement
    if (m_jitterMovement){
        x = x + (( 2 * m_radius * drand48() ) - m_radius) * m_amount;
        y = y + (( 2 * m_radius * drand48() ) - m_radius) * m_amount;
    }

    m_pixelSize = dev->colorSpace()->pixelSize();
    m_inkColor = color;
    m_counter++;

    qint32 pixelSize = dev->colorSpace()->pixelSize();
    KisRandomAccessor accessor = dev->createRandomAccessor((int)x, (int)y);

    QHash<QString, QVariant> params;
    params["h"] = 0.0;
    params["s"] = 0.0;
    params["v"] = 0.0;

    KoColorTransformation* transfo = dev->colorSpace()->createColorTransformation("hsv_adjustment", params);
    transfo->transform(m_inkColor.data(), m_inkColor.data(), 1);

    int points = (m_coverage * (M_PI * m_radius * m_radius) );

    qreal nx, ny;
    int ix, iy;

    int opacity = 255;
    m_inkColor.setOpacity(opacity);

    for (int i = 0;i<points;i++){
        nx = (drand48() * m_radius * 2) - m_radius;
        ny = (drand48() * m_radius * 2) - m_radius;

        qreal distance = sqrt(nx*nx + ny*ny);
        qreal weight = distance / m_radius; 

        if ( (distance+0.5) > m_radius){
            continue;
        }

        
        if (m_useParticles){
            paintParticle(accessor,m_inkColor,nx + x, ny + y);
        }
        else
        {
            ix = (int)(nx + x + 0.5);
            iy = (int)(ny + y + 0.5);
            accessor.moveTo(ix, iy);
/* 
            int srcAlpha = dev->colorSpace()->alpha ( accessor.rawData() );
            int dstAlpha = (255 * (1.0-weight));
            dstAlpha = (dstAlpha + srcAlpha) % 255;
            m_inkColor.setOpacity(dstAlpha);
*/
            memcpy(accessor.rawData(), m_inkColor.data(), pixelSize);
        }


    }

    m_radius = tmpRadius;

}

SprayBrush::~SprayBrush()
{
}


void SprayBrush::paintParticle(KisRandomAccessor &writeAccessor,const KoColor &color,qreal rx, qreal ry){
    qreal MAX_OPACITY = 255;
    // opacity top left, right, bottom left, right
    KoColor pcolor(color);

    int ipx = int ( rx );
    int ipy = int ( ry );   
    qreal fx = rx - ipx;
    qreal fy = ry - ipy;

    int btl = ( 1-fx ) * ( 1-fy ) * MAX_OPACITY;
    int btr = ( fx )  * ( 1-fy ) * MAX_OPACITY;
    int bbl = ( 1-fx ) * ( fy )  * MAX_OPACITY;
    int bbr = ( fx )  * ( fy )  * MAX_OPACITY;

    pcolor.setOpacity ( btl );
    writeAccessor.moveTo ( ipx  , ipy );
    memcpy ( writeAccessor.rawData(), pcolor.data(), m_pixelSize );

    pcolor.setOpacity ( btr );
    writeAccessor.moveTo ( ipx + 1, ipy );
    memcpy ( writeAccessor.rawData(), pcolor.data(), m_pixelSize );

    pcolor.setOpacity ( bbl );
    writeAccessor.moveTo ( ipx, ipy + 1 );
    memcpy ( writeAccessor.rawData(), pcolor.data(), m_pixelSize );

    pcolor.setOpacity ( bbr );
    writeAccessor.moveTo ( ipx + 1, ipy + 1 );
    memcpy ( writeAccessor.rawData(), pcolor.data(), m_pixelSize );
}

