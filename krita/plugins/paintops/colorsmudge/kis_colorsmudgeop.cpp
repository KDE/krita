/*
 *  Copyright (C) 2011 Silvio Heinrich <plassy@web.de>
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

#include "kis_colorsmudgeop.h"

#include <QRect>

#include <KoColorSpaceRegistry.h>
#include <KoColorTransformation.h>
#include <KoColor.h>
#include <KoCompositeOp.h>

#include <kis_brush.h>
#include <kis_global.h>
#include <kis_paint_device.h>
#include <kis_painter.h>
#include <kis_image.h>
#include <kis_selection.h>
#include <kis_brush_based_paintop_settings.h>

KisColorSmudgeOp::KisColorSmudgeOp(const KisBrushBasedPaintOpSettings* settings, KisPainter* painter, KisImageWSP image):
    KisBrushBasedPaintOp(settings, painter),
    m_firstRun(true), m_tempDev(0), m_image(image),
    m_smudgeRateOption("SmudgeRate"), m_colorRateOption("ColorRate")
{
    Q_ASSERT(settings);
    Q_ASSERT(painter);
    
    m_sizeOption.readOptionSetting(settings);
    m_spacingOption.readOptionSetting(settings);
    m_smudgeRateOption.readOptionSetting(settings);
    m_colorRateOption.readOptionSetting(settings);
    m_mergedPaintOption.readOptionSetting(settings);
    m_rotationOption.readOptionSetting(settings);
    m_scatterOption.readOptionSetting(settings);
    
    m_sizeOption.sensor()->reset();
    m_spacingOption.sensor()->reset();
    m_smudgeRateOption.sensor()->reset();
    m_colorRateOption.sensor()->reset();
    m_rotationOption.sensor()->reset();
    m_scatterOption.sensor()->reset();

    m_tempDev     = new KisPaintDevice(painter->device()->colorSpace());
    m_tempPainter = new KisPainter(m_tempDev);
}

KisColorSmudgeOp::~KisColorSmudgeOp()
{
    delete m_tempPainter;
}

qreal KisColorSmudgeOp::paintAt(const KisPaintInformation& info)
{
    // Simple error catching
    if (!painter()->device())
        return 1.0;
    
    KisBrushSP brush = m_brush;
    
    if(!brush || !brush->canPaintFor(info))
        return 1.0;
    
    // get the scaling factor calculated by the size option
    qreal scale    = m_sizeOption.apply(info);
    qreal rotation = m_rotationOption.apply(info);
    
    // don't paint anything if the brush is too samll
    if((scale*brush->width()) <= 0.01 || (scale*brush->height()) <= 0.01)
        return 1.0;
    
    setCurrentScale(scale);
    setCurrentRotation(rotation);
    
    QPointF scatteredPos = m_scatterOption.apply(info, qreal(brush->width() + brush->height()) / 2.0 * scale);
    QPointF point        = scatteredPos - brush->hotSpot(scale, scale, rotation);
    
    qint32 x, y;
    qreal xFraction, yFraction; // will not be used
    splitCoordinate(point.x(), &x, &xFraction);
    splitCoordinate(point.y(), &y, &yFraction);
    
    KisFixedPaintDeviceSP maskDab = 0;
    
    // Extract the brush mask (maskDab) from brush with the correct scaled size
    if(brush->brushType() == IMAGE || brush->brushType() == PIPE_IMAGE) {
        // This is for bitmap brushes
        maskDab = brush->paintDevice(painter()->device()->colorSpace(), scale, rotation, info, 0.0, 0.0);
    } else {
        // This is for parametric brushes, those created in the Autobrush popup config dialogue
        maskDab = cachedDab();
        brush->mask(maskDab, painter()->paintColor(), scale, scale, rotation, info, 0.0, 0.0);
    }
    
    // transforms the fixed paint device with the current brush to alpha color space
    // to use it as an alpha/transparency mask
    maskDab->convertTo(KoColorSpaceRegistry::instance()->alpha8());
    
    // save the old opacity value and composite mode
    quint8               oldOpacity = painter()->opacity();
    const KoCompositeOp* oldMode    = painter()->compositeOp();
    qreal                fpOpacity  = qreal(oldOpacity) / 255.0;
    
    if(!m_firstRun) {
        // set opacity calculated by the rate option
        m_smudgeRateOption.apply(*painter(), info, 0.0, 1.0, fpOpacity);
        
        // then blit the temporary painting device on the canvas at the current brush position
        // the alpha mask (maskDab) will be used here to only blit the pixels that are in the area (shape) of the brush
        painter()->setCompositeOp(COMPOSITE_COPY);
        painter()->bitBltWithFixedSelection(x, y, m_tempDev, maskDab, maskDab->bounds().width(), maskDab->bounds().height());
    }
    else m_firstRun = false;
    
    if(m_image && m_mergedPaintOption.isChecked()) {
        m_tempPainter->setCompositeOp(COMPOSITE_COPY);
        m_tempPainter->setOpacity(OPACITY_OPAQUE_U8);
        m_image->lock();
        m_tempPainter->bitBlt(0, 0, m_image->projection(), x, y, brush->width(), brush->width());
        m_image->unlock();
    }
    else {
        // IMPORTANT: clear the temporary painting device to color black with zero opacity
        //            it will only clear the extents of the brush
        m_tempDev->clear(QRect(0, 0, brush->width(), brush->height()));
    }
    
    // reset composite mode and opacity
    // then cut out the area from the canvas under the brush
    // and blit it to the temporary painting device
    m_tempPainter->setCompositeOp(COMPOSITE_OVER);
    m_tempPainter->setOpacity(OPACITY_OPAQUE_U8);
    m_tempPainter->bitBlt(0, 0, painter()->device(), x, y, brush->width(), brush->width());
    
    // if the user selected the color smudge option
    // we will mix some color into the temorary painting device (m_tempDev)
    if(m_colorRateOption.isChecked()) {
        // this will apply the opacy (selected by the user) to copyPainter
        // (but fit the rate inbetween the range 0.0 to (1.0-SmudgeRate))
        qreal maxColorRate = qMax<qreal>(1.0-m_smudgeRateOption.getRate(), 0.2);
        m_colorRateOption.apply(*m_tempPainter, info, 0.0, maxColorRate, fpOpacity);
        
        // paint a rectangle with the current color (foreground color)
        // into the temporary painting device and use the user selected
        // composite mode
        m_tempPainter->setCompositeOp(oldMode);
        m_tempPainter->setFillStyle(KisPainter::FillStyleForegroundColor);
        m_tempPainter->setPaintColor(painter()->paintColor());
        m_tempPainter->paintRect(maskDab->bounds());
    }
    
    // restore orginal opacy and composite mode values
    painter()->setOpacity(oldOpacity);
    painter()->setCompositeOp(oldMode);
    
    if(m_spacingOption.isChecked())
        return spacing(m_spacingOption.apply(info));
    
    return spacing(scale);
}
