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

#include <cmath>
#include <memory>
#include <QRect>

#include <KoColorSpaceRegistry.h>
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
    m_smudgeRateOption("SmudgeRate"),
    m_colorRateOption("ColorRate"),
    m_smudgeAccessor(painter->device()->createRandomAccessorNG(0, 0))
{
    Q_ASSERT(settings);
    Q_ASSERT(painter);
    
    m_sizeOption.readOptionSetting(settings);
    m_opacityOption.readOptionSetting(settings);
    m_spacingOption.readOptionSetting(settings);
    m_smudgeRateOption.readOptionSetting(settings);
    m_colorRateOption.readOptionSetting(settings);
    m_overlayModeOption.readOptionSetting(settings);
    m_rotationOption.readOptionSetting(settings);
    m_scatterOption.readOptionSetting(settings);
    m_gradientOption.readOptionSetting(settings);
    
    m_sizeOption.sensor()->reset();
    m_opacityOption.sensor()->reset();
    m_spacingOption.sensor()->reset();
    m_smudgeRateOption.sensor()->reset();
    m_colorRateOption.sensor()->reset();
    m_rotationOption.sensor()->reset();
    m_scatterOption.sensor()->reset();
    m_gradientOption.sensor()->reset();
    
    m_gradient    = painter->gradient();
    m_tempDev     = new KisPaintDevice(painter->device()->colorSpace());
    m_tempPainter = new KisPainter(m_tempDev);
}

KisColorSmudgeOp::~KisColorSmudgeOp()
{
    delete m_tempPainter;
}

void KisColorSmudgeOp::updateMask(const KisPaintInformation& info, double scale, double rotation)
{
    // Extract the brush mask (m_maskDab) from brush with the correct scaled size
    if(m_brush->brushType() == IMAGE || m_brush->brushType() == PIPE_IMAGE) {
        // This is for bitmap brushes
        m_maskDab    = m_brush->paintDevice(painter()->device()->colorSpace(), scale, rotation, info, 0.0, 0.0);
        m_maskBounds = m_maskDab->bounds();
    } else {
        // This is for parametric brushes, those created in the Autobrush popup config dialogue
        const static qint32 MAX_SIZE_DIFF = 1;
        const static double MAX_ROT_DIFF  = 5.0 * M_PI/180.0;
        
        qint32  width           = m_brush->maskWidth(scale, rotation);
        qint32  height          = m_brush->maskHeight(scale, rotation);
        quint32 index           = m_brush->brushIndex(info);
        double  angle           = m_brush->maskAngle(rotation);
        bool    rotationChanged = qAbs(angle-m_angle) > MAX_ROT_DIFF;
        bool    sizeChanged     = qAbs(width-m_maskBounds.width()) > MAX_SIZE_DIFF || qAbs(height-m_maskBounds.height()) > MAX_SIZE_DIFF;
        bool    indexChanged    = index != m_brushIndex;
        
        if(rotationChanged || m_maskDab.isNull())
            m_angle = angle;
        
        // calculate a new brush mask only if the size or rotation of the brush changed significantly
        if(sizeChanged || rotationChanged || m_maskDab.isNull() || indexChanged) {
            m_maskDab = cachedDab();
            m_brush->mask(m_maskDab, painter()->paintColor(), scale, scale, rotation, info, 0.0, 0.0);
            m_maskBounds = m_maskDab->bounds();
            m_brushIndex = index;
        }
    }
    
    // transforms the fixed paint device with the current brush
    // to alpha color space to use it as an alpha/transparency mask
    m_maskDab->convertTo(KoColorSpaceRegistry::instance()->alpha8(), KoColorConversionTransformation::IntentPerceptual, KoColorConversionTransformation::BlackpointCompensation);
}

qreal KisColorSmudgeOp::paintAt(const KisPaintInformation& info)
{
    KisBrushSP brush = m_brush;
    
    // Simple error catching
    if(!painter()->device() || !brush || !brush->canPaintFor(info))
        return 1.0;
    
    // get the scaling factor calculated by the size option
    qreal scale    = m_sizeOption.apply(info);
    qreal rotation = m_rotationOption.apply(info);
    qreal diagonal = std::sqrt((qreal)brush->width()*brush->width() + brush->height()*brush->height());
    
    // don't paint anything if the brush is too samll
    if((scale*brush->width()) <= 0.01 || (scale*brush->height()) <= 0.01)
        return 1.0;
    
    setCurrentScale(scale);
    setCurrentRotation(rotation);
    
    QPointF scatteredPos = m_scatterOption.apply(info, diagonal*scale);
    QPointF point        = scatteredPos - brush->hotSpot(scale, scale, rotation);
    
    qint32 x, y;
    qreal xFraction, yFraction; // will not be used
    splitCoordinate(point.x(), &x, &xFraction);
    splitCoordinate(point.y(), &y, &yFraction);
    
    // save the old opacity value and composite mode
    quint8               oldOpacity = painter()->opacity();
    const KoCompositeOp* oldMode    = painter()->compositeOp();
    qreal                fpOpacity  = (qreal(oldOpacity) / 255.0) * m_opacityOption.getOpacityf(info);
    
    if(!m_firstRun) {
        // if color is disabled (only smudge) and "overlay mode is enabled
        // then first blit the region under the brush from the image projection
        // to the painting device to prevent a rapid build up of alpha value
        // if the color to be smudged is semi transparent
        if(m_image && m_overlayModeOption.isChecked() && !m_colorRateOption.isChecked()) {
            painter()->setCompositeOp(COMPOSITE_COPY);
            painter()->setOpacity(OPACITY_OPAQUE_U8);
            m_image->blockUpdates();
            painter()->bitBlt(x, y, m_image->projection(), x, y, m_maskBounds.width(), m_maskBounds.height());
            m_image->unblockUpdates();
        }
        
        // set opacity calculated by the rate option
        m_smudgeRateOption.apply(*painter(), info, 0.0, 1.0, fpOpacity);
        
        // then blit the temporary painting device on the canvas at the current brush position
        // the alpha mask (maskDab) will be used here to only blit the pixels that are in the area (shape) of the brush
        painter()->setCompositeOp(COMPOSITE_COPY);
        painter()->bitBltWithFixedSelection(x, y, m_tempDev, m_maskDab, m_maskBounds.width(), m_maskBounds.height());
    }
    else m_firstRun = false;
    
    // update the brush mask if needed. the mask is then stored in m_maskDab
    // and the extends of the mask is stored in m_maskBounds
    updateMask(info, scale, rotation);

    if(m_image && m_overlayModeOption.isChecked()) {
        m_tempPainter->setCompositeOp(COMPOSITE_COPY);
        m_tempPainter->setOpacity(OPACITY_OPAQUE_U8);
        m_image->blockUpdates();
        m_tempPainter->bitBlt(0, 0, m_image->projection(), x, y, m_maskBounds.width(), m_maskBounds.height());
        m_image->unblockUpdates();
    }
    else {
        // IMPORTANT: clear the temporary painting device to color black with zero opacity
        //            it will only clear the extents of the brush
        m_tempDev->clear(m_maskBounds);
    }
    
    // reset composite mode and opacity
    m_tempPainter->setCompositeOp(COMPOSITE_OVER);
    m_tempPainter->setOpacity(OPACITY_OPAQUE_U8);
    
    if(m_smudgeRateOption.getMode() == KisSmudgeOption::SMEARING_MODE) {
        // cut out the area from the canvas under the brush
        // and blit it to the temporary painting device
        m_tempPainter->bitBlt(0, 0, painter()->device(), x, y, m_maskBounds.width(), m_maskBounds.height());
    }
    else {
        KoColorSpace* cs    = painter()->device()->colorSpace();
        qint32        px    = x + m_maskBounds.width()  / 2;
        qint32        py    = y + m_maskBounds.height() / 2;
        // get the pixel on the canvas that lies beneath the center
        // of the dab and fill  the temporary paint device with that color
        m_smudgeAccessor->moveTo(px, py);
        m_tempPainter->fill(0, 0, m_maskBounds.width(), m_maskBounds.height(), KoColor(m_smudgeAccessor->rawData(), cs));
    }
    
    // if the user selected the color smudge option
    // we will mix some color into the temporary painting device (m_tempDev)
    if(m_colorRateOption.isChecked()) {
        // this will apply the opacy (selected by the user) to copyPainter
        // (but fit the rate inbetween the range 0.0 to (1.0-SmudgeRate))
        qreal maxColorRate = qMax<qreal>(1.0-m_smudgeRateOption.getRate(), 0.2);
        m_colorRateOption.apply(*m_tempPainter, info, 0.0, maxColorRate, fpOpacity);
        
        // paint a rectangle with the current color (foreground color)
        // or a gradient color (if enabled)
        // into the temporary painting device and use the user selected
        // composite mode
        KoColor color = painter()->paintColor();
        m_gradientOption.apply(color, m_gradient, info);
        m_tempPainter->setCompositeOp(oldMode);
        m_tempPainter->fill(0, 0, m_maskBounds.width(), m_maskBounds.height(), color);
    }
    
    // restore orginal opacy and composite mode values
    painter()->setOpacity(oldOpacity);
    painter()->setCompositeOp(oldMode);
    
    if(m_spacingOption.isChecked())
        return spacing(m_spacingOption.apply(info));
    
    return spacing(scale);
}
