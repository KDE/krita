/*
 *  Copyright (c) 2009,2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "kis_soft_paintop.h"
#include "kis_soft_paintop_settings.h"

#include <cmath>
#include <math.h>

#include <QRect>


#include <kis_image.h>
#include <kis_debug.h>

#include <kis_brush.h>
#include <kis_global.h>
#include <kis_paint_device.h>
#include <kis_painter.h>
#include <kis_types.h>
#include <kis_paintop.h>
#include <kis_selection.h>
#include <kis_random_accessor.h>

#include <KoColor.h>

KisSoftPaintOp::KisSoftPaintOp(const KisSoftPaintOpSettings *settings, KisPainter * painter, KisImageWSP image)
    : KisPaintOp( painter )
    , m_settings( settings )
    , m_image ( image )
{
    m_radius = qRound(0.5 * m_settings->diameter());
    m_color = painter->paintColor();

    m_xSpacing = qMax(1.0,m_settings->spacing() * m_radius);
    m_ySpacing = qMax(1.0,m_settings->spacing() * m_radius);
    m_spacing = sqrt(m_xSpacing*m_xSpacing + m_ySpacing*m_ySpacing);

    
    m_distMask = new KisCircleAlphaMask(m_radius);
    m_distMask->setSigma( m_settings->sigma(), m_settings->flow() / 100.0  );
    m_distMask->generateGaussMap( true );
    
    qreal start = m_settings->start();
    qreal end = m_settings->end();
    //m_distMask->smooth( start,end );


#ifdef BENCHMARK
    m_count = m_total = 0;
#endif
}

KisSoftPaintOp::~KisSoftPaintOp()
{
    delete m_distMask;
}


double KisSoftPaintOp::spacing(double& xSpacing, double& ySpacing, double pressure1, double pressure2) const
{
    Q_UNUSED(pressure1);
    Q_UNUSED(pressure2);
    xSpacing = m_xSpacing;
    ySpacing = m_ySpacing;
    return m_spacing;
}


void KisSoftPaintOp::paintAt(const KisPaintInformation& info)
{
#ifdef BENCHMARK
    QTime time;
    time.start();
#endif

    if (!painter()) return;

    if (!m_dab) {
        m_dab = new KisPaintDevice(painter()->device()->colorSpace());
    }
    else {
        m_dab->clear();
    }

    KisRandomAccessor acc = m_dab->createRandomAccessor( info.pos().x(), info.pos().y() );

   
    quint8 alpha;
    int pixelSize = m_dab->colorSpace()->pixelSize();

    int curX = qRound(info.pos().x());
    int curY = qRound(info.pos().y());
    int left = curX - m_distMask->radius();
    int top = curY - m_distMask->radius();

    int w = m_distMask->radius() * 2 + 1;
    int h = w;

    int newX;
    int newY;

    KisRectIterator m_srcIt = m_dab->createRectIterator(left, top, w ,h );
    int x;
    int y;

    int border = ( m_distMask->radius() + 1 ) * ( m_distMask->radius() + 1 );    
    for (;!m_srcIt.isDone(); ++m_srcIt) {

        x = m_srcIt.x();
        y = m_srcIt.y();

        newX = x - curX;
        newY = y - curY;
/*        if (newX*newX + newY*newY > border)
        {
            continue;
        }else*/
        {
            alpha = qRound(m_distMask->valueAt(newX,newY) * OPACITY_OPAQUE);
            if (alpha == 0) continue;
            m_color.setOpacity(alpha);
            memcpy(m_srcIt.rawData(), m_color.data(), pixelSize);
        }
    }
    

    QRect rc(left,top,w,h); 
    painter()->bitBlt(rc.x(), rc.y(), m_dab, rc.x(), rc.y(), rc.width(), rc.height());

#ifdef BENCHMARK
    int msec = time.elapsed();
    kDebug() << msec << " ms/dab " << "[average: " << m_total / (qreal)m_count << "]";
    m_total += msec;
    m_count++;
#endif

}



