/*
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#ifndef KIS_CURVE_MASK_H
#define KIS_CURVE_MASK_H

// Qt 
class QRectF;
#include <QVector>

// KOffice
class KoColor;

// Krita
class KisFixedPaintDevice;
#include <kis_cubic_curve.h>

#include <kis_types.h>
#include <kis_brush_size_option.h>

struct KisCurveProperties{
    QVector<qreal> curveData;
    KisCubicCurve curve;
    bool controlByPressure;
};


class KisCurveMask{
    
public:
    KisCurveMask();
    ~KisCurveMask(){};
    void mask(KisFixedPaintDeviceSP dab, const KoColor &color, qreal scale, qreal rotation, qreal xSubpixel, qreal ySubpixel);
    
    void setProperties(KisCurveProperties * properties){
        m_properties = properties;
    }
    
    void setSizeProperties(KisBrushSizeProperties * properties){
        m_sizeProperties = properties;
    }

    QPointF hotSpot(qreal scale, qreal rotation);
    
    qreal maskWidth(qreal scale){
        return m_sizeProperties->diameter * scale;
    }
    
    qreal maskHeight(qreal scale){
        return m_sizeProperties->diameter * m_sizeProperties->aspect  * scale;
    }
    

private:    
    KisCurveProperties * m_properties;
    KisBrushSizeProperties * m_sizeProperties;
    
    qreal m_fWidth;
    qreal m_fHeight;
    qreal m_majorAxis;
    qreal m_minorAxis;
    qreal m_inverseScale;
    QRectF m_maskRect;
    
    inline qreal valueAt(qreal x, qreal y);
    // used for ellipse canonical implicit equation (x/a)^2 + (y/b)^2 = 1
    inline qreal norme(qreal x,qreal y){
        return x*x + y*y; 
    }
    
};

#endif
