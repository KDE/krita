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
#include <kis_fixed_paint_device.h>


KisColorSmudgeOp::KisColorSmudgeOp(const KisBrushBasedPaintOpSettings* settings, KisPainter* painter, KisImageWSP image):
    KisBrushBasedPaintOp(settings, painter),
    m_firstRun(true), m_image(image),
    m_tempDev(painter->device()->createCompositionSourceDevice()),
    m_backgroundPainter(new KisPainter(m_tempDev)),
    m_smudgePainter(new KisPainter(m_tempDev)),
    m_colorRatePainter(new KisPainter(m_tempDev)),
    m_smudgeRateOption("SmudgeRate"),
    m_colorRateOption("ColorRate"),
    m_colorPicker(painter->device(), painter->paintColor())
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

    m_gradient = painter->gradient();

    m_backgroundPainter->setCompositeOp(COMPOSITE_COPY);
    // Smudge Painter works in default COMPOSITE_OVER mode
    m_colorRatePainter->setCompositeOp(painter->compositeOp()->id());
}

KisColorSmudgeOp::~KisColorSmudgeOp()
{
    delete m_backgroundPainter;
    delete m_colorRatePainter;
    delete m_smudgePainter;
}

void KisColorSmudgeOp::updateMask(const KisPaintInformation& info, double scale, double rotation)
{
    static const KoColorSpace *cs = KoColorSpaceRegistry::instance()->alpha8();
    static KoColor color(Qt::black, cs);

    m_maskDab = m_dabCache->fetchDab(cs,
                                     color,
                                     scale, scale,
                                     rotation,
                                     info);

    m_maskBounds = m_maskDab->bounds();
}

inline void KisColorSmudgeOp::getTopLeftAligned(const QPointF &pos, const QPointF &hotSpot, qint32 *x, qint32 *y)
{
    QPointF topLeft = pos - hotSpot;

    qreal xFraction, yFraction; // will not be used
    splitCoordinate(topLeft.x(), x, &xFraction);
    splitCoordinate(topLeft.y(), y, &yFraction);
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
    QPointF hotSpot      = brush->hotSpot(scale, scale, rotation, info);

    qint32 dstX, dstY;
    getTopLeftAligned(scatteredPos, hotSpot, &dstX, &dstY);

    qint32 srcX, srcY;
    getTopLeftAligned(m_lastPaintPos, hotSpot, &srcX, &srcY);

    // Save the hot spot point for the next iteration
    m_lastPaintPos = scatteredPos;

    qreal coveredDistance = m_spacingOption.isChecked() ?
        spacing(m_spacingOption.apply(info)) : spacing(scale);

    if (m_firstRun) {
        m_firstRun = false;
        return coveredDistance;
    }


    // save the old opacity value and composite mode
    quint8               oldOpacity = painter()->opacity();
    QString              oldModeId = painter()->compositeOp()->id();
    qreal                fpOpacity  = (qreal(oldOpacity) / 255.0) * m_opacityOption.getOpacityf(info);

    // update the brush mask if needed. the mask is then stored in m_maskDab
    // and the extends of the mask is stored in m_maskBounds
    updateMask(info, scale, rotation);

    if(m_image && m_overlayModeOption.isChecked()) {
        m_image->blockUpdates();
        m_backgroundPainter->bitBlt(0, 0, m_image->projection(), srcX, srcY, m_maskBounds.width(), m_maskBounds.height());
        m_image->unblockUpdates();
    }
    else {
        // IMPORTANT: clear the temporary painting device to color black with zero opacity
        //            it will only clear the extents of the brush
        m_tempDev->clear(m_maskBounds);
    }

    if(m_smudgeRateOption.getMode() == KisSmudgeOption::SMEARING_MODE) {
        // cut out the area from the canvas under the brush
        // and blit it to the temporary painting device
        m_smudgePainter->bitBlt(0, 0, painter()->device(), srcX, srcY, m_maskBounds.width(), m_maskBounds.height());
    }
    else {
        QPoint pt = (QPoint(srcX, srcY) + hotSpot).toPoint();
        // get the pixel on the canvas that lies beneath the hot spot
        // of the dab and fill  the temporary paint device with that color

        KoColor color = painter()->paintColor();
        m_colorPicker.pickColor(pt.x(), pt.y(), color.data());
        m_smudgePainter->fill(0, 0, m_maskBounds.width(), m_maskBounds.height(), color);
    }

    // if the user selected the color smudge option
    // we will mix some color into the temporary painting device (m_tempDev)
    if(m_colorRateOption.isChecked()) {
        // this will apply the opacy (selected by the user) to copyPainter
        // (but fit the rate inbetween the range 0.0 to (1.0-SmudgeRate))
        qreal maxColorRate = qMax<qreal>(1.0-m_smudgeRateOption.getRate(), 0.2);
        m_colorRateOption.apply(*m_colorRatePainter, info, 0.0, maxColorRate, fpOpacity);

        // paint a rectangle with the current color (foreground color)
        // or a gradient color (if enabled)
        // into the temporary painting device and use the user selected
        // composite mode
        KoColor color = painter()->paintColor();
        m_gradientOption.apply(color, m_gradient, info);
        m_colorRatePainter->fill(0, 0, m_maskBounds.width(), m_maskBounds.height(), color);
    }


    // if color is disabled (only smudge) and "overlay mode is enabled
    // then first blit the region under the brush from the image projection
    // to the painting device to prevent a rapid build up of alpha value
    // if the color to be smudged is semi transparent
    if(m_image && m_overlayModeOption.isChecked() && !m_colorRateOption.isChecked()) {
        painter()->setCompositeOp(COMPOSITE_COPY);
        painter()->setOpacity(OPACITY_OPAQUE_U8);
        m_image->blockUpdates();
        painter()->bitBlt(dstX, dstY, m_image->projection(), dstX, dstY, m_maskBounds.width(), m_maskBounds.height());
        m_image->unblockUpdates();
    }

    // set opacity calculated by the rate option
    m_smudgeRateOption.apply(*painter(), info, 0.0, 1.0, fpOpacity);

    // then blit the temporary painting device on the canvas at the current brush position
    // the alpha mask (maskDab) will be used here to only blit the pixels that are in the area (shape) of the brush
    painter()->setCompositeOp(COMPOSITE_COPY);
    painter()->bitBltWithFixedSelection(dstX, dstY, m_tempDev, m_maskDab, m_maskBounds.width(), m_maskBounds.height());

    // restore orginal opacy and composite mode values
    painter()->setOpacity(oldOpacity);
    painter()->setCompositeOp(oldModeId);

    return coveredDistance;
}
