/*
 *  SPDX-FileCopyrightText: 2008, 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "deform_brush.h"
#include "kis_painter.h"

#include "kis_fixed_paint_device.h"

#include <KoColor.h>
#include <KoColorSpace.h>

#include <QRect>

#include <kis_types.h>
#include <kis_iterator_ng.h>
#include <kis_cross_device_color_sampler.h>

#include <cmath>
#include <ctime>
#include <KoColorSpaceRegistry.h>

const qreal degToRad = M_PI / 180.0;


DeformBrush::DeformBrush()
{
    m_firstPaint = false;
    m_counter = 1;
    m_deformAction = 0;
}

DeformBrush::~DeformBrush()
{
    delete m_deformAction;
}

void DeformBrush::initDeformAction()
{
    DeformModes mode = DeformModes(m_properties->deform_action - 1);

    switch (mode) {
    case GROW:
    case SHRINK: {
        m_deformAction = new DeformScale();
        break;
    }
    case SWIRL_CW:
    case SWIRL_CCW: {
        m_deformAction = new DeformRotation();
        break;
    }

    case MOVE: {
        m_deformAction = new DeformMove();
        static_cast<DeformMove*>(m_deformAction)->setFactor(m_properties->deform_amount);
        break;
    }
    case LENS_IN:
    case LENS_OUT: {
        m_deformAction = new DeformLens();
        static_cast<DeformLens*>(m_deformAction)->setLensFactor(m_properties->deform_amount, 0.0);
        static_cast<DeformLens*>(m_deformAction)->setMode(mode == LENS_OUT);
        break;
    }
    case DEFORM_COLOR: {
        m_deformAction = new DeformColor();
        static_cast<DeformColor*>(m_deformAction)->setFactor(m_properties->deform_amount);
        break;
    }
    default: {
        m_deformAction = new DeformBase();
        break;
    }
    }
}

bool DeformBrush::setupAction(
    DeformModes mode, const QPointF& pos, QTransform const& rotation)
{

    switch (mode) {
    case GROW:
    case SHRINK: {
        // grow or shrink, the sign decide
        qreal sign = (mode == GROW) ? 1.0 : -1.0;
        qreal factor;
        if (m_properties->deform_use_counter) {
            factor = (1.0 + sign * (m_counter * m_counter / 100.0));
        }
        else {
            factor = (1.0 + sign * (m_properties->deform_amount));
        }
        dynamic_cast<DeformScale*>(m_deformAction)->setFactor(factor);
        break;
    }
    case SWIRL_CW:
    case SWIRL_CCW: {
        // CW or CCW, the sign decide
        qreal sign = (mode == SWIRL_CW) ? 1.0 : -1.0;
        qreal factor;
        if (m_properties->deform_use_counter) {
            factor = m_counter * sign * degToRad;
        }
        else {
            factor = (360 * m_properties->deform_amount * 0.5) * sign * degToRad;
        }
        dynamic_cast<DeformRotation*>(m_deformAction)->setAlpha(factor);
        break;
    }
    case MOVE: {
        if (m_firstPaint == false) {
            m_prevX = pos.x();
            m_prevY = pos.y();
            static_cast<DeformMove*>(m_deformAction)->setDistance(0.0, 0.0);
            m_firstPaint = true;
            return false;
        }
        else {
            qreal xDistance = pos.x() - m_prevX;
            qreal yDistance = pos.y() - m_prevY;
            rotation.map(xDistance, yDistance, &xDistance, &yDistance);
            static_cast<DeformMove*>(m_deformAction)->setDistance(xDistance, yDistance);
            m_prevX = pos.x();
            m_prevY = pos.y();
        }
        break;
    }
    case LENS_IN:
    case LENS_OUT: {
        static_cast<DeformLens*>(m_deformAction)->setMaxDistance(m_sizeProperties->brush_diameter * 0.5, m_sizeProperties->brush_diameter * 0.5);
        break;
    }
    case DEFORM_COLOR: {
        // no run-time setup
        break;
    }
    default: {
        break;
    }
    }
    return true;
}

KisFixedPaintDeviceSP DeformBrush::paintMask(KisFixedPaintDeviceSP dab,
        KisPaintDeviceSP layer,
        KisRandomSourceSP randomSource,
        qreal scale,
        qreal rotation,
        QPointF pos, qreal subPixelX, qreal subPixelY, int dabX, int dabY)
{
    KisFixedPaintDeviceSP mask = new KisFixedPaintDevice(KoColorSpaceRegistry::instance()->alpha8());
    KisCrossDeviceColorSampler colorSampler(layer, dab);

    qreal fWidth = maskWidth(scale);
    qreal fHeight = maskHeight(scale);

    int dstWidth =  qRound(m_maskRect.width());
    int dstHeight = qRound(m_maskRect.height());

    // clear
    if (dab->bounds().width() != dstWidth || dab->bounds().height() != dstHeight) {
        dab->setRect(m_maskRect.toRect());
        dab->lazyGrowBufferWithoutInitialization();
    }

    qreal const centerX = dstWidth  * 0.5  + subPixelX;
    qreal const centerY = dstHeight * 0.5  + subPixelY;

    qreal const majorAxis = 2.0 / fWidth;
    qreal const minorAxis = 2.0 / fHeight;

    qreal distance;

    QTransform forwardRotationMatrix;
    forwardRotationMatrix.rotate(rotation);
    QTransform reverseRotationMatrix;
    reverseRotationMatrix.rotate(-rotation);

    // if can't paint, stop
    if (!setupAction(DeformModes(m_properties->deform_action - 1),
                     pos, forwardRotationMatrix))
    {
        return 0;
    }

    mask->setRect(dab->bounds());
    mask->lazyGrowBufferWithoutInitialization();
    quint8* maskPointer = mask->data();
    qint8 maskPixelSize = mask->pixelSize();

    quint8* dabPointer = dab->data();
    int dabPixelSize = dab->colorSpace()->pixelSize();

    for (int y = 0; y <  dstHeight; y++) {
        for (int x = 0; x < dstWidth; x++) {
            qreal maskX = x - centerX;
            qreal maskY = y - centerY;
            forwardRotationMatrix.map(maskX, maskY, &maskX, &maskY);
            distance = norme(maskX * majorAxis, maskY * minorAxis);

            if (distance > 1.0) {
                // leave there OPACITY TRANSPARENT pixel (default pixel)

                colorSampler.sampleOldColor(x + dabX, y + dabY, dabPointer);
                dabPointer += dabPixelSize;

                *maskPointer = OPACITY_TRANSPARENT_U8;
                maskPointer += maskPixelSize;
                continue;
            }

            if (m_sizeProperties->brush_density != 1.0) {
                if (m_sizeProperties->brush_density < randomSource->generateNormalized()) {
                    dabPointer += dabPixelSize;
                    *maskPointer = OPACITY_TRANSPARENT_U8;
                    maskPointer += maskPixelSize;
                    continue;
                }
            }

            m_deformAction->transform(&maskX, &maskY, distance, randomSource);
            reverseRotationMatrix.map(maskX, maskY, &maskX, &maskY);

            maskX += pos.x();
            maskY += pos.y();

            if (!m_properties->deform_use_bilinear) {
                maskX = qRound(maskX);
                maskY = qRound(maskY);
            }

            if (m_properties->deform_use_old_data) {
                colorSampler.sampleOldColor(maskX, maskY, dabPointer);
            }
            else {
                colorSampler.sampleColor(maskX, maskY, dabPointer);
            }

            dabPointer += dabPixelSize;

            *maskPointer = OPACITY_OPAQUE_U8;
            maskPointer += maskPixelSize;

        }
    }
    m_counter++;

    return mask;

}

void DeformBrush::debugColor(const quint8* data, KoColorSpace * cs)
{
    QColor rgbcolor;
    cs->toQColor(data, &rgbcolor);
    dbgPlugins << "RGBA: ("
               << rgbcolor.red()
               << ", " << rgbcolor.green()
               << ", " << rgbcolor.blue()
               << ", " << rgbcolor.alpha() << ")";
}

QPointF DeformBrush::hotSpot(qreal scale, qreal rotation)
{
    qreal fWidth = maskWidth(scale);
    qreal fHeight = maskHeight(scale);

    QTransform m;
    m.reset();
    m.rotate(-rotation);

    m_maskRect = QRect(0, 0, fWidth, fHeight);
    m_maskRect.translate(-m_maskRect.center());
    m_maskRect = m.mapRect(m_maskRect);
    m_maskRect.translate(-m_maskRect.topLeft());
    return m_maskRect.center();
}
