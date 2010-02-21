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

#include <kis_softop_option.h>


KisSoftPaintOp::KisSoftPaintOp(const KisSoftPaintOpSettings *settings, KisPainter * painter, KisImageWSP image)
    : KisPaintOp( painter )
    , m_settings( settings )
    , m_image ( image )
{

    m_brushType = SoftBrushType(settings->getInt(SOFT_BRUSH_TIP));
    // brushType == 0
    if (m_brushType == CURVE){
       
        m_curveMaskProperties.diameter = quint16(settings->getDouble(SOFTCURVE_DIAMETER));
        m_curveMaskProperties.scale = settings->getDouble(SOFTCURVE_SCALE);    
        m_curveMaskProperties.curve = settings->getCubicCurve(SOFTCURVE_CURVE);
        
        m_radius = qRound(0.5 * m_curveMaskProperties.diameter);
        m_curveMaskProperties.curveData = m_curveMaskProperties.curve.floatTransfer(m_radius+2);
        
        m_curveMask.setProperties(&m_curveMaskProperties);
        
        m_gaussBrush.distMask = 0;
    }else if (m_brushType == GAUSS){

        m_radius = qRound(settings->getDouble(SOFT_DIAMETER) * 0.5);
        m_gaussBrush.distMask = new KisCircleAlphaMask(m_radius);
        m_gaussBrush.distMask->setSigma( settings->getDouble(SOFT_SIGMA), settings->getDouble(SOFT_SOFTNESS) / 100.0  );
        m_gaussBrush.distMask->generateGaussMap( true );
        qreal start = m_settings->getDouble(SOFT_START);
        qreal end = m_settings->getDouble(SOFT_END);
        m_gaussBrush.distMask->smooth( start,end );
    }
    m_color = painter->paintColor();

        
    m_xSpacing = qMax(1.0,settings->getDouble(SOFT_SPACING) * m_radius);
    m_ySpacing = qMax(1.0,settings->getDouble(SOFT_SPACING) * m_radius);
    m_spacing = sqrt(m_xSpacing*m_xSpacing + m_ySpacing*m_ySpacing);
    
#ifdef BENCHMARK
    m_count = m_total = 0;
#endif
}

KisSoftPaintOp::~KisSoftPaintOp()
{
    delete m_gaussBrush.distMask;
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

    quint8 alpha = 0;
    int pixelSize = m_dab->colorSpace()->pixelSize();

    int curX = qRound(info.pos().x());
    int curY = qRound(info.pos().y());

    int left = curX - m_radius;
    int top = curY - m_radius;

    int w = m_radius * 2 + 1;
    int h = w;

    int maskX;
    int maskY;

    KisRectIterator m_srcIt = m_dab->createRectIterator(left, top, w ,h );
    int x;
    int y;

    
    
    if (m_brushType == CURVE){
        KisFixedPaintDeviceSP dab = cachedDab(painter()->device()->colorSpace());

        qint32 x;
        double subPixelX;
        qint32 y;
        double subPixelY;

        QPointF pos = info.pos() - m_curveMask.hotSpot(m_curveMaskProperties.scale);
        
        splitCoordinate(pos.x(), &x, &subPixelX);
        splitCoordinate(pos.y(), &y, &subPixelY);

        m_curveMask.mask(dab,painter()->paintColor(),m_curveMaskProperties.scale,0.0,subPixelX,subPixelY);
        painter()->bltFixed(QPoint(x, y), dab, dab->bounds());
        return;
    }
    
    
    int border = ( m_radius ) * ( m_radius );    
    for (;!m_srcIt.isDone(); ++m_srcIt) {

        x = m_srcIt.x();
        y = m_srcIt.y();

        maskX = x - curX;
        maskY = y - curY;
        qreal dist = maskX*maskX + maskY*maskY;
        if (dist > border)
        {
            //continue;
        }else
        {
            if (m_brushType == GAUSS)
            {
                alpha = qRound(m_gaussBrush.distMask->valueAt(maskX,maskY) * OPACITY_OPAQUE_U8);
            }
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



