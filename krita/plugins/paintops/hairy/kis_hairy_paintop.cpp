/*
 *  Copyright (c) 2008-2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "kis_hairy_paintop.h"
#include "kis_hairy_paintop_settings.h"

#include <cmath>
#include <QRect>

#include <kis_image.h>
#include <kis_debug.h>

#include "kis_paint_device.h"
#include "kis_painter.h"
#include <kis_vec.h>

#include "brush_shape.h"

#include <kis_hairy_ink_option.h>
#include <kis_hairy_shape_option.h>
#include <kis_hairy_bristle_option.h>
#include <kis_brush_option.h>
#include <kis_brush_based_paintop_settings.h>

#include "kis_brush.h"

KisHairyPaintOp::KisHairyPaintOp(const
                               KisBrushBasedPaintOpSettings *settings, KisPainter * painter, KisImageWSP image)
        : KisPaintOp(painter)
        , m_settings(settings)
        , m_image(image)
        , newStrokeFlag(true)

{
    Q_ASSERT(settings);

    KisBrushOption brushOption;
    brushOption.readOptionSetting(settings);
    KisBrushSP kisBrush = brushOption.brush();
    BrushShape bs; 
    
    bs.setColorSpace(painter->device()->colorSpace());
    bs.setHasColor(kisBrush->brushType() != MASK);
    bs.fromQImageWithDensity(kisBrush->image(), settings->getDouble(HAIRY_BRISTLE_DENSITY) * 0.01);
    m_brush.setBrushShape(bs);
    
    loadSettings(settings);
    m_brush.setProperties( &m_properties );
    
    if (!bs.hasColor()){
        m_brush.setInkColor(painter->paintColor());    
    }
    
    if (!settings->node()) {
        m_dev = 0;
    } else {
        m_dev = settings->node()->paintDevice();
    }
    
    m_sizeOption.readOptionSetting(settings);
    m_sizeOption.sensor()->reset();
}

void KisHairyPaintOp::loadSettings(const KisBrushBasedPaintOpSettings* settings)
{
/*    m_properties.radius = settings->getInt(HAIRY_RADIUS);
    m_properties.sigma = settings->getDouble(HAIRY_SIGMA);
    m_properties.isbrushDimension1D = settings->getBool(HAIRY_IS_DIMENSION_1D);*/
    
    m_properties.inkAmount = settings->getInt(HAIRY_INK_AMOUNT);
    //TODO: wait for the transfer function with variable size
    QList<float> list;
    KisCubicCurve curve = settings->getCubicCurve(HAIRY_INK_DEPLETION_CURVE);
    for (int i=0;i < m_properties.inkAmount;i++){
        list << curve.value( i/qreal(m_properties.inkAmount-1) );
    }
    curve.floatTransfer(m_properties.inkAmount);

    m_properties.inkDepletionCurve = list;

    m_properties.useSaturation = settings->getBool(HAIRY_INK_USE_SATURATION);
    m_properties.useOpacity = settings->getBool(HAIRY_INK_USE_OPACITY);
    m_properties.useWeights = settings->getBool(HAIRY_INK_USE_WEIGHTS);

    m_properties.pressureWeight = settings->getDouble(HAIRY_INK_PRESSURE_WEIGHT) / 100.0;
    m_properties.bristleLengthWeight = settings->getDouble(HAIRY_INK_BRISTLE_LENGTH_WEIGHT) / 100.0;
    m_properties.bristleInkAmountWeight = settings->getDouble(HAIRY_INK_BRISTLE_INK_AMOUNT_WEIGHT) / 100.0;
    m_properties.inkDepletionWeight = settings->getDouble(HAIRY_INK_DEPLETION_WEIGHT);
    m_properties.useSoakInk = settings->getBool(HAIRY_INK_SOAK);

    m_properties.useMousePressure = settings->getBool(HAIRY_BRISTLE_USE_MOUSEPRESSURE);
    m_properties.shearFactor = settings->getDouble(HAIRY_BRISTLE_SHEAR);
    m_properties.randomFactor = settings->getDouble(HAIRY_BRISTLE_RANDOM);
    m_properties.scaleFactor = settings->getDouble(HAIRY_BRISTLE_SCALE);
    m_properties.threshold = settings->getBool(HAIRY_BRISTLE_THRESHOLD);
    m_properties.antialias = settings->getBool(HAIRY_BRISTLE_ANTI_ALIASING);
    m_properties.useCompositing = settings->getBool(HAIRY_BRISTLE_USE_COMPOSITING);
    m_properties.connectedPath = settings->getBool(HAIRY_BRISTLE_CONNECTED);

    
    /*    BrushShape brushShape;
    if (m_properties.isbrushDimension1D) 
    {
        brushShape.fromLine(m_properties.radius, m_properties.sigma);
        brushShape.thresholdBristles(0.1);
    } else {
        brushShape.fromGaussian(m_properties.radius, m_properties.sigma);
        brushShape.thresholdBristles(0.1);
    }
    m_brush.setBrushShape(brushShape);*/
    
}


double KisHairyPaintOp::paintAt(const KisPaintInformation& info)
{
    Q_UNUSED(info);
    return 0.5;
}


KisDistanceInformation KisHairyPaintOp::paintLine(const KisPaintInformation &pi1, const KisPaintInformation &pi2, const KisDistanceInformation& savedDist)
{
    // spacing is ignored in hairy-e, maybe todo
    Q_UNUSED(savedDist);

    if (!painter()) return KisDistanceInformation();

    if (!m_dab) {
        m_dab = new KisPaintDevice(painter()->device()->colorSpace());
    } else {
        m_dab->clear();
    }
    
    qreal scale = m_properties.scaleFactor * KisPaintOp::scaleForPressure(m_sizeOption.apply(pi2));
    
    m_brush.paintLine(m_dab, m_dev, pi1, pi2, scale);

    //QRect rc = m_dab->exactBounds();
    QRect rc = m_dab->extent();
    painter()->bitBlt(rc.topLeft(), m_dab, rc);
    
    KisVector2D end = toKisVector2D(pi2.pos());
    KisVector2D start = toKisVector2D(pi1.pos());
    KisVector2D dragVec = end - start;
    return KisDistanceInformation(0, dragVec.norm());
}
