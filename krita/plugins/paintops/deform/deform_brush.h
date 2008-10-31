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

#ifndef _DEFORM_BRUSH_H_
#define _DEFORM_BRUSH_H_

#include <QVector>
#include <QList>

#include <KoColor.h>

#include <kis_paint_device.h>
#include <kis_paint_information.h>
#include <kis_random_accessor.h>
#include <kis_image.h>

class DeformBrush
{

public:
    DeformBrush();
    ~DeformBrush();
    void paint(KisPaintDeviceSP dev,KisPaintDeviceSP layer, const KisPaintInformation &info);
    void paintLine(KisPaintDeviceSP dev,KisPaintDeviceSP layer, const KisPaintInformation &pi1, const KisPaintInformation &pi2);

    void bilinear_interpolation(double x, double y );

    void scale(qreal cursorX,qreal cursorY, qreal factor);
    void swirl(qreal cursorX,qreal cursorY, qreal alpha);

    void setRadius( int deformRadius ){
        m_radius = deformRadius;
    }
    void setDeformAmount ( qreal deformAmount ){
        m_amount = deformAmount;
    }
    void setInterpolation( bool useBilinear ){
        m_useBilinear = useBilinear;
    }
    void setAction( int deformAction ){
        m_action = deformAction;
    }
    void setImage( KisImageSP image ){
        m_image = image;
    }

    void setCounter(int value){
        m_counter = value;
    }



private:
    bool point_interpolation( qreal* x, qreal* y, KisImageSP image );
    void debugColor(const quint8* data);

    // width and height for interpolation
    KisImageSP m_image;

    // temporary device
    KisPaintDeviceSP m_dev;
    KisRandomAccessor * m_readAccessor;
    KisRandomAccessor * m_writeAccessor;
    quint32 m_pixelSize;
    
    //temporary KoColor for optimalization in bilinear interpolation
    KoColor * m_tempColor;

    int m_radius;
    qreal m_amount;
    bool m_useBilinear;
    int m_action;

    int m_counter;
};

#endif
