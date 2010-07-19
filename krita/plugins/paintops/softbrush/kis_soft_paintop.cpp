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
#include <time.h>

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
#include <kis_brush_size_option.h>
#include "kis_hsv_option.h"

#if defined(_WIN32) || defined(_WIN64)
#define srand48 srand
inline double drand48()
{
    return double(rand()) / RAND_MAX;
}
#endif

KisSoftPaintOp::KisSoftPaintOp(const KisSoftPaintOpSettings *settings, KisPainter * painter, KisImageWSP image)
    : KisPaintOp( painter )
    , m_settings( settings )
    , m_image ( image )
{
    // custom options
    m_hsvProperties.readOptionSetting(settings);
    m_sizeProperties.readOptionSetting(settings);
    // sensors
    m_sizeOption.readOptionSetting(settings);
    m_opacityOption.readOptionSetting(settings);
    m_rotationOption.readOptionSetting(settings);
    m_sizeOption.sensor()->reset();
    m_opacityOption.sensor()->reset();
    m_rotationOption.sensor()->reset();
    
    m_radius = qRound(0.5 * m_sizeProperties.diameter);
    m_amount = 0.0;
    m_brushType = SoftBrushType(settings->getInt(SOFT_BRUSH_TIP));

    if (m_brushType == CURVE){
        srand48(time(0));
        
        m_curveMaskProperties.controlByPressure = settings->getBool(SOFTCURVE_CONTROL_BY_PRESSURE);
        m_curveMaskProperties.curve = settings->getCubicCurve(SOFTCURVE_CURVE);
        m_curveMaskProperties.curveData = m_curveMaskProperties.curve.floatTransfer(m_radius+2);
        m_points = m_curveMaskProperties.curve.points();
        
        m_curveMask.setProperties(&m_curveMaskProperties);
        m_curveMask.setSizeProperties(&m_sizeProperties);

        m_gaussBrush.distMask = 0;
    }else if (m_brushType == GAUSS){
        m_gaussBrush.distMask = new KisCircleAlphaMask(m_radius);
        m_gaussBrush.distMask->setSigma( settings->getDouble(SOFT_SIGMA), settings->getDouble(SOFT_SOFTNESS) / 100.0  );
        m_gaussBrush.distMask->generateGaussMap( true );
        qreal start = m_settings->getDouble(SOFT_START);
        qreal end = m_settings->getDouble(SOFT_END);
        m_gaussBrush.distMask->smooth( start,end );
    }
    m_color = painter->paintColor();

    // compute spacing for brush
    m_xSpacing = qMax(qreal(0.5),m_sizeProperties.spacing * m_sizeProperties.diameter * m_sizeProperties.scale);
    m_ySpacing = qMax(qreal(0.5),m_sizeProperties.spacing * m_sizeProperties.diameter * m_sizeProperties.aspect * m_sizeProperties.scale);
    m_spacing = qMax(m_xSpacing, m_ySpacing);
    
#ifdef BENCHMARK
    m_count = m_total = 0;
#endif
}

KisSoftPaintOp::~KisSoftPaintOp()
{
    delete m_gaussBrush.distMask;
}

double KisSoftPaintOp::paintAt(const KisPaintInformation& info)
{
    if (!painter()) return m_spacing;

    m_color = painter()->paintColor();

    if (m_hsvProperties.enabled){
        QHash<QString, QVariant> params;
        params["h"] = 0.0;
        params["s"] = 0.0;
        params["v"] = 0.0;
        
        switch (m_hsvProperties.hmode){
            case 0:
                params["h"] = m_hsvProperties.hcurve.value( m_amount / (qreal)m_hsvProperties.hAmount ) ;
                break;
            case 1:
                params["h"] = -m_hsvProperties.hcurve.value( m_amount / (qreal)m_hsvProperties.hAmount ) ;
                break;
            case 2:
                params["h"] = drand48() * 2.0 - 1.0;
            default: break;
            case 3:
                params["h"] = scaleForPressure(info.pressure())*0.5;
            break;
        }
    
        switch (m_hsvProperties.smode){
            case 0:
                params["s"] = m_hsvProperties.scurve.value( m_amount / (qreal)m_hsvProperties.sAmount ) ;
                break;
            case 1:
                params["s"] = -m_hsvProperties.scurve.value( m_amount / (qreal)m_hsvProperties.sAmount ) ;
                break;
            case 2:
                params["s"] = drand48() * 2.0 - 1.0;
                break;
            case 3:
                params["s"] = scaleForPressure(info.pressure())*0.5;
                break;
        }
        
        switch (m_hsvProperties.vmode){
            case 0:
                params["v"] = m_hsvProperties.vcurve.value( m_amount / (qreal)m_hsvProperties.vAmount ) ;
                break;
            case 1:
                params["v"] = -m_hsvProperties.vcurve.value( m_amount / (qreal)m_hsvProperties.vAmount ) ;
                break;
            case 2:
                params["v"] = drand48() * 2.0 - 1.0;
            default: break;
            case 3:
                params["v"] = scaleForPressure(info.pressure())*0.5;
            break;

        }

        KoColorTransformation* transfo;
        transfo = painter()->device()->colorSpace()->createColorTransformation("hsv_adjustment", params);
        transfo->transform(m_color.data(), m_color.data() , 1);
        m_amount += 1.0;
    }
    
    if (m_brushType == CURVE){
        KisFixedPaintDeviceSP dab = cachedDab(painter()->device()->colorSpace());

        qint32 x;
        double subPixelX;
        qint32 y;
        double subPixelY;

        QPointF pt = info.pos();
        if (m_sizeProperties.jitterEnabled){
            pt.setX(pt.x() + (  ( m_sizeProperties.diameter * drand48() ) - m_radius) * m_sizeProperties.jitterMovementAmount);
            pt.setY(pt.y() + (  ( m_sizeProperties.diameter * drand48() ) - m_radius) * m_sizeProperties.jitterMovementAmount);
        }
    
        if ( m_curveMaskProperties.controlByPressure ){
            transformSoftness( info.pressure() );
        }

        qreal scale = m_sizeProperties.scale * KisPaintOp::scaleForPressure(m_sizeOption.apply(info));
        qreal rotation = m_sizeProperties.rotation + m_rotationOption.apply(info);
        
        QPointF pos = pt - m_curveMask.hotSpot(scale, rotation);
        
        splitCoordinate(pos.x(), &x, &subPixelX);
        splitCoordinate(pos.y(), &y, &subPixelY);
        
        m_curveMask.mask(dab,m_color,scale,rotation,subPixelX,subPixelY);
        quint8 origOpacity = m_opacityOption.apply(painter(), info);
        QRect rc = dab->bounds();
        painter()->bltFixed(x,y,dab,rc.x(), rc.y(), rc.width(),rc.height());
        painter()->setOpacity(origOpacity);
        return m_spacing;
    }

    if (!m_dab) {
        m_dab = new KisPaintDevice(painter()->device()->colorSpace());
    }
    else {
        m_dab->clear();
    }

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
    return m_spacing;
}

void KisSoftPaintOp::transformSoftness(qreal scaleY)
{
    int size = m_curveMaskProperties.curve.points().size();
    QPointF tmp;
    for (int i = 0; i < size;i++){
        tmp = m_points.at(i);
        tmp.ry() *= scaleY;
        m_curveMaskProperties.curve.setPoint(i,tmp);
    }
    m_curveMaskProperties.curveData = m_curveMaskProperties.curve.floatTransfer(m_radius+2);
}

