/*
 *  Copyright (c) 2008, 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#ifndef _DEFORM_BRUSH_H_
#define _DEFORM_BRUSH_H_

#include <kis_paint_device.h>
#include <kis_paint_information.h>

#include <kis_brush_size_properties.h>

class DeformProperties
{
public:
    int action;
    qreal deformAmount;
    bool useBilinear;
    bool useCounter;
    bool useOldData;
};


class DeformBrush
{

public:
    DeformBrush();
    ~DeformBrush();
    
    void paint(KisFixedPaintDeviceSP dab, KisPaintDeviceSP layer,qreal scale,qreal rotation,QPointF pos,qreal subPixelX,qreal subPixelY);
    void setSizeProperties(KisBrushSizeProperties * properties){
        m_sizeProperties = properties;
    }

    void setProperties(DeformProperties * properties){
        m_properties = properties;
    }

    QPointF hotSpot(){
        return QPointF(m_sizeProperties->diameter * 0.5,m_sizeProperties->diameter * 0.5);
    }


private:
    void maskScale(KisFixedPaintDeviceSP dab, KisPaintDeviceSP layer, 
                   qreal scale, qreal rotation, QPointF pos,qreal subPixelX, qreal subPixelY,
                   qreal factor);

    void maskSwirl(KisFixedPaintDeviceSP dab, KisPaintDeviceSP layer, 
                   qreal scale, qreal rotation, QPointF pos,qreal subPixelX, qreal subPixelY,
                   qreal factor);

    void maskMove(KisFixedPaintDeviceSP dab, KisPaintDeviceSP layer, 
                   qreal scale, qreal rotation, QPointF pos,qreal subPixelX, qreal subPixelY,
                   qreal dx, qreal dy);

    void maskLensDistortion(KisFixedPaintDeviceSP dab, KisPaintDeviceSP layer, 
                   qreal scale, qreal rotation, QPointF pos,qreal subPixelX, qreal subPixelY,
                   qreal k1, qreal k2);

    void maskDeformColor(KisFixedPaintDeviceSP dab, KisPaintDeviceSP layer, 
                   qreal scale, qreal rotation, QPointF pos,qreal subPixelX, qreal subPixelY,
                   qreal amount);
                   

    /// move pixel from new computed coords newX, newY to x,y (inverse mapping)
    void movePixel(qreal newX, qreal newY, quint8 *dst);

    void debugColor(const quint8* data, KoColorSpace * cs);
    //void precomputeDistances(int radius);

//     void bilinear_interpolation(double x, double y , quint8 *dst);
//     void bilinear_interpolation_old(double x, double y , quint8 *dst);

private:
    KisRandomSubAccessorPixel * m_srcAcc;

    qreal* m_distanceTable;
    // == radius
    qreal m_maxdist;

    bool m_firstPaint;
    qreal m_prevX, m_prevY;
    int m_counter;
    quint32 m_pixelSize;
    
    DeformProperties * m_properties;
    KisBrushSizeProperties * m_sizeProperties;
};

#endif
