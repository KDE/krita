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
    srand48(time(0));
}

SprayBrush::SprayBrush()
{
    m_radius = 0;
    m_counter = 0;
}



void SprayBrush::paint(KisPaintDeviceSP dev, qreal x, qreal y, const KoColor &color)
{
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

	qreal angle;
    qreal lengthX;
    qreal lengthY;
    for (int i = 0;i<points;i++){
		angle = drand48();
        // different X and Y length??
		lengthY = lengthX = drand48();
        // I hope we live the are where sin and cos is not slow for spray
        nx = (sin(angle * M_PI * 2) * m_radius * lengthX);
        ny = (cos(angle * M_PI * 2) * m_radius * lengthY);

        if (m_useParticles)
        {
            paintParticle(accessor,m_inkColor,nx + x, ny + y);
        }
        else
        {
            ix = qRound(nx + x);
            iy = qRound(ny + y);
            accessor.moveTo(ix, iy);
            memcpy(accessor.rawData(), m_inkColor.data(), pixelSize);
        }


    }
    // recover from jittering
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

    int btl = qRound( ( 1-fx ) * ( 1-fy ) * MAX_OPACITY );
    int btr = qRound( ( fx )  * ( 1-fy ) * MAX_OPACITY );
    int bbl = qRound( ( 1-fx ) * ( fy )  * MAX_OPACITY );
    int bbr = qRound( ( fx )  * ( fy )  * MAX_OPACITY );

    // this version overwrite pixels, e.g. when it sprays two particle next
    // to each other, the pixel with lower opacity can override other pixel.
    // Maybe some kind of compositing using here would be cool

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

