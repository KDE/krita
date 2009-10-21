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
    void paint(KisPaintDeviceSP dev, KisPaintDeviceSP layer, const KisPaintInformation &info);

    void bilinear_interpolation(double x, double y , quint8 *dst);
    void bilinear_interpolation_old(double x, double y , quint8 *dst);

    void scale(qreal cursorX, qreal cursorY, qreal factor);
    void swirl(qreal cursorX, qreal cursorY, qreal alpha);
    void move(qreal cursorX, qreal cursorY, qreal dx, qreal dy);

    void fastMove(qreal cursorX, qreal cursorY, qreal dx, qreal dy);
    void fastSwirl(qreal cursorX, qreal cursorY, qreal alpha);
    void fastDeformColor(qreal cursorX, qreal cursorY, qreal amount);
    void fastLensDistortion(qreal cursorX, qreal cursorY, qreal k1, qreal k2);

    void lensDistortion(qreal cursorX, qreal cursorY, qreal k1, qreal k2);
    void deformColor(qreal cursorX, qreal cursorY, qreal amount);

    void setRadius(int deformRadius) {
        m_radius = deformRadius;

        m_maxdist = sqrt(pow(m_radius, 2));
        precomputeDistances(m_radius);
    }

    void setDeformAmount(qreal deformAmount) {
        m_amount = deformAmount;
    }

    void setInterpolation(bool useBilinear) {
        m_useBilinear = useBilinear;
    }

    void setAction(int deformAction) {
        m_action = deformAction;
    }

    void setImage(KisImageWSP image) {
        m_image = image;
    }

    void setCounter(int value) {
        m_counter = value;
    }

    void setUseCounter(bool useCounter) {
        m_useCounter = useCounter;
    }

    void setUseOldData(bool useOldData) {
        m_useOldData = useOldData;
    }

private:
    qreal distanceFromCenter(int x, int y) {
        return m_distanceTable[y*(m_radius+1)+x];
    }

    /// move pixel from new computed coords newX, newY to x,y (inverse mapping)
    void movePixel(qreal newX, qreal newY, quint8 *dst);
    void myMovePixel(qreal newX, qreal newY, quint8 *dst);

    bool point_interpolation(qreal* x, qreal* y, KisImageWSP image);
    void debugColor(const quint8* data);
    void precomputeDistances(int radius);
    void fastScale(qreal cursorX, qreal cursorY, qreal factor);

    // width and height for interpolation
    KisImageWSP m_image;

    // temporary device
    KisPaintDeviceSP m_dev;
    KisPaintDeviceSP m_dab;

    KisRandomConstAccessor * m_readAccessor;
    KisRandomAccessor * m_writeAccessor;
    quint32 m_pixelSize;

    KisRandomSubAccessorPixel * m_srcAcc;

    //temporary KoColor for optimalization in bilinear interpolation
    KoColor * m_tempColor;

    qreal* m_distanceTable;

    int m_radius;
    qreal m_maxdist;
    qreal m_amount;
    bool m_useBilinear;
    int m_action;

    int m_counter;

    bool m_firstPaint;
    bool m_useCounter;
    bool m_useOldData;
    qreal m_prevX, m_prevY;
};

#endif
