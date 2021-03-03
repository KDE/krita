/*
 *  SPDX-FileCopyrightText: 2008, 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _DEFORM_BRUSH_H_
#define _DEFORM_BRUSH_H_

#include <kis_paint_device.h>
#include <brushengine/kis_paint_information.h>

#include <kis_brush_size_option.h>
#include <kis_deform_option.h>
#include "kis_algebra_2d.h"

#include <time.h>

#if defined(_WIN32) || defined(_WIN64)
#define srand48 srand
inline double drand48()
{
    return double(rand()) / RAND_MAX;
}
#endif

enum DeformModes {GROW, SHRINK, SWIRL_CW, SWIRL_CCW, MOVE, LENS_IN, LENS_OUT, DEFORM_COLOR};

class DeformProperties
{
public:
    int action {0};
    qreal deformAmount {0.0};
    bool useBilinear {false};
    bool useCounter {false};
    bool useOldData {false};
};


class DeformBase
{
public:
    DeformBase() {}
    virtual ~DeformBase() {}
    virtual void transform(qreal * x, qreal * y, qreal distance, KisRandomSourceSP randomSource) {
        Q_UNUSED(x);
        Q_UNUSED(y);
        Q_UNUSED(distance);
        Q_UNUSED(randomSource);
    }
};

/// Inverse weighted inverse scaling - grow&shrink
class DeformScale : public DeformBase
{

public:
    void setFactor(qreal factor) {
        m_factor = factor;
    }
    qreal factor() {
        return m_factor;
    }
    void transform(qreal* x, qreal* y, qreal distance, KisRandomSourceSP randomSource) override {
        Q_UNUSED(randomSource);
        qreal scaleFactor = KisAlgebra2D::signPZ(m_factor) * (qAbs((1.0 - distance) * m_factor) + distance);
        *x = *x / scaleFactor;
        *y = *y / scaleFactor;
    }

private:
    qreal m_factor {0.0};
};

/// Inverse weighted rotation - swirlCW&&swirlCWW
class DeformRotation : public DeformBase
{

public:
    void setAlpha(qreal alpha) {
        m_alpha = alpha;
    }
    void transform(qreal* maskX, qreal* maskY, qreal distance, KisRandomSourceSP randomSource) override {
        Q_UNUSED(randomSource);
        distance = 1.0 - distance;
        qreal rotX = cos(-m_alpha * distance) * (*maskX) - sin(-m_alpha * distance) * (*maskY);
        qreal rotY = sin(-m_alpha * distance) * (*maskX) + cos(-m_alpha * distance) * (*maskY);

        *maskX = rotX;
        *maskY = rotY;
    }

private:
    qreal m_alpha {0.0};
};

/// Inverse move
class DeformMove : public DeformBase
{
public:
    void setFactor(qreal factor) {
        m_factor = factor;
    }
    void setDistance(qreal dx, qreal dy) {
        m_dx = dx;
        m_dy = dy;
    }
    void transform(qreal* maskX, qreal* maskY, qreal distance, KisRandomSourceSP randomSource) override {
        Q_UNUSED(randomSource);
        *maskX -= m_dx * m_factor * (1.0 - distance);
        *maskY -= m_dy * m_factor * (1.0 - distance);
    }

private:
    qreal m_dx {0.0};
    qreal m_dy {0.0};
    qreal m_factor {0.0};
};

/// Inverse lens distortion
class DeformLens : public DeformBase
{
public:
    void setLensFactor(qreal k1, qreal k2) {
        m_k1 = k1;
        m_k2 = k2;
    }
    void setMaxDistance(qreal maxX, qreal maxY) {
        m_maxX = maxX;
        m_maxY = maxY;
    }
    void setMode(bool out) {
        m_out = out;
    }

    void transform(qreal* maskX, qreal* maskY, qreal distance, KisRandomSourceSP randomSource) override {
        Q_UNUSED(distance);
        Q_UNUSED(randomSource);
        //normalize
        qreal normX = *maskX / m_maxX;
        qreal normY = *maskY / m_maxY;

        qreal radius_2 = normX * normX  + normY * normY;
        qreal radius_4 = radius_2 * radius_2;

        if (m_out) {
            *maskX = normX * (1.0 + m_k1 * radius_2 + m_k2 * radius_4);
            *maskY = normY * (1.0 + m_k1 * radius_2 + m_k2 * radius_4);
        }
        else {
            *maskX = normX / (1.0 + m_k1 * radius_2 + m_k2 * radius_4);
            *maskY = normY / (1.0 + m_k1 * radius_2 + m_k2 * radius_4);
        }

        *maskX = m_maxX * (*maskX);
        *maskY = m_maxY * (*maskY);
    }

private:
    qreal m_k1 {0.0}, m_k2 {0.0};
    qreal m_maxX {0.0}, m_maxY {0.0};
    bool m_out {false};
};

/// Randomly disturb the pixels
class DeformColor : public DeformBase
{
public:
    DeformColor() {
    }

    void setFactor(qreal factor) {
        m_factor = factor;
    }
    void transform(qreal* x, qreal* y, qreal distance, KisRandomSourceSP randomSource) override {
        Q_UNUSED(distance);
        qreal randomX = m_factor * ((randomSource->generateNormalized() * 2.0) - 1.0);
        qreal randomY = m_factor * ((randomSource->generateNormalized() * 2.0) - 1.0);
        *x += randomX;
        *y += randomY;
    }

private:
    qreal m_factor {0.0};
};





class DeformBrush
{

public:
    DeformBrush();
    ~DeformBrush();

    KisFixedPaintDeviceSP paintMask(KisFixedPaintDeviceSP dab, KisPaintDeviceSP layer, KisRandomSourceSP randomSource,
                                    qreal scale, qreal rotation, QPointF pos,
                                    qreal subPixelX, qreal subPixelY, int dabX, int dabY);

    void setSizeProperties(KisBrushSizeOptionProperties * properties) {
        m_sizeProperties = properties;
    }
    void setProperties(DeformOption * properties) {
        m_properties = properties;
    }
    void initDeformAction();
    QPointF hotSpot(qreal scale, qreal rotation);

private:
    // return true if can paint
    bool setupAction(
        DeformModes mode, const QPointF& pos, QTransform const& rotation);
    void debugColor(const quint8* data, KoColorSpace * cs);

    qreal maskWidth(qreal scale) {
        return m_sizeProperties->brush_diameter * scale;
    }

    qreal maskHeight(qreal scale) {
        return m_sizeProperties->brush_diameter * m_sizeProperties->brush_aspect  * scale;
    }

    inline qreal norme(qreal x, qreal y) {
        return x * x + y * y;
    }


private:
    KisRandomSubAccessorSP m_srcAcc;
    bool m_firstPaint {false};
    qreal m_prevX {0.0}, m_prevY {0.0};
    int m_counter {1}; // taken from the constructor

    QRectF m_maskRect;

    DeformBase * m_deformAction {0};

    DeformOption * m_properties {0};
    KisBrushSizeOptionProperties * m_sizeProperties {0};
};


#endif
