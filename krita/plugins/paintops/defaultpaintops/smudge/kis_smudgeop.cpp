/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004-2008 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Clarence Dang <dang@kde.org>
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2010 José Luis Vergara Toloza <pentalis@gmail.com>
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

#include "kis_smudgeop.h"

#include <QRect>

#include <kis_debug.h>

#include <KoColorSpaceRegistry.h>
#include <KoColorTransformation.h>
#include <KoColor.h>
#include <KoCompositeOp.h>

#include <kis_brush.h>
#include <kis_global.h>
#include <kis_paint_device.h>
#include <kis_painter.h>
#include <kis_selection.h>
#include <kis_brush_based_paintop_settings.h>


// Both limits defined to be 15 units away from the min (0) or max (255) to allow actual mixing of colors
const quint8 MIXABLE_UPPER_LIMIT = 240;
const quint8 MIXABLE_LOWER_LIMIT = 15;
// All pieces of color extracted from the canvas will be centered around MIGHTY_CENTER
const QPoint MIGHTY_CENTER = QPoint(0, 0);

KisSmudgeOp::KisSmudgeOp(const KisBrushBasedPaintOpSettings *settings, KisPainter *painter, KisImageWSP image)
        : KisBrushBasedPaintOp(settings, painter)
        , m_firstRun(true)
        , m_srcdev(0)
{
    Q_UNUSED(image);
    Q_ASSERT(settings);
    Q_ASSERT(painter);
    m_sizeOption.readOptionSetting(settings);
    m_opacityOption.readOptionSetting(settings);
    m_rateOption.readOptionSetting(settings);
    m_sizeOption.sensor()->reset();
    m_opacityOption.sensor()->reset();
    m_rateOption.sensor()->reset();

    m_srcdev = new KisPaintDevice(painter->device()->colorSpace());
}

KisSmudgeOp::~KisSmudgeOp()
{
}

/* To smudge, one does the following:
 * at first, initialize a temporary paint device with a copy of the original (dab-sized piece, really).
 * all other times:
   reduce the transparency of the temporary paint device so as to let it mix gradually
 * combine the temp device with the piece the brush currently is 'painting', according to a mix (opacity)
   note that in the first step, this does the actual copying of the data
 * this combination is then composited upon the actual image
 * TODO: what happened exactly in 1.6 (and should happen now) when the dab resizes halfway due to pressure?
 * ---> This version of smudge attempts to center new 'absorbed' dabs over the previous ones, as well as
   make the areas not visible to the user gradually vanish. (Commented blocks at the end allow changes
   to this behavior)
*/
    
double KisSmudgeOp::paintAt(const KisPaintInformation& info)
{
    KisBrushSP brush = m_brush;
    
    // Simple error catching
    if (!painter()->device()) return 1.0;
    if (!brush) return 1.0;
    if (!brush->canPaintFor(info)) return 1.0;

    // Grow the brush (this includes the mask) according to pressure or other parameters
    double scale = KisPaintOp::scaleForPressure(m_sizeOption.apply(info));
    if ((scale * brush->width()) <= 0.01 || (scale * brush->height()) <= 0.01) return 1.0;
    setCurrentScale(scale);
    
    /* Align a point that represents the top-left corner of the brush stroke rendering
    with the mouse pointer and take into account the brush mask size */
    QPointF hotSpot = brush->hotSpot(scale, scale);
    QPointF pt = info.pos() - hotSpot;

    // Split the coordinates into integer plus fractional parts. The integer
    // is where the dab will be positioned and the fractional part determines
    // the sub-pixel positioning.
    qint32 x;
    double xFraction;
    qint32 y;
    double yFraction;

    splitCoordinate(pt.x(), &x, &xFraction);
    splitCoordinate(pt.y(), &y, &yFraction);

    KisFixedPaintDeviceSP maskDab = 0;

    if (brush->brushType() == IMAGE || brush->brushType() == PIPE_IMAGE) {
        maskDab = brush->paintDevice(painter()->device()->colorSpace(), scale, 0.0, info, xFraction, yFraction);
        maskDab->convertTo(KoColorSpaceRegistry::instance()->alpha8());
    } else {
        maskDab = cachedDab();
        KoColor color = painter()->paintColor();
        color.convertTo(maskDab->colorSpace());
        brush->mask(maskDab, color, scale, scale, 0.0, info, xFraction, yFraction);
        maskDab->convertTo(KoColorSpaceRegistry::instance()->alpha8());
    }

    /*-----Convenient renaming for the limits of the maskDab------*/
    qint32 sw = maskDab->bounds().width();
    qint32 sh = maskDab->bounds().height();
    
    // Prepare the top left corner of the temporary paint device where the extracted color will be drawn
    QPoint extractionTopLeft = QPoint(MIGHTY_CENTER.x() - sw / 2,
                                      MIGHTY_CENTER.y() - sh / 2);
                             
    /* In the block below, the opacity of the color stored in m_srcdev TODO: rename it
    is reduced in opacity. Nothing of the color present in it is left out*/
    int opacity = OPACITY_OPAQUE_U8;
    if (!m_firstRun) {
        opacity = m_rateOption.apply(opacity, info);
        /* Without those limits, the smudge brush doesn't smudge anymore, it either makes a single
        dropplet of color, or drags a frame indefinitely over the canvas. */
        if (opacity > MIXABLE_UPPER_LIMIT) opacity = MIXABLE_UPPER_LIMIT;
        if (opacity < MIXABLE_LOWER_LIMIT) opacity = MIXABLE_LOWER_LIMIT;
        
        // This is the whole temporary data area
        QRect wholeTempData = m_srcdev->extent();
        KisRectIterator it = m_srcdev->createRectIterator(wholeTempData.x(), wholeTempData.y(),
                                                          wholeTempData.width(), wholeTempData.height());
        KoColorSpace* cs = m_srcdev->colorSpace();
        while (!it.isDone()) {
            cs->setOpacity(it.rawData(), quint8(cs->opacityF(it.rawData()) * opacity), 1);
            ++it;
        }
        
        // Invert the opacity value for color absorption in the next lines (copyPainter)
        opacity = OPACITY_OPAQUE_U8 - opacity;
    }
    else {
        m_firstRun = false;
    }

    // copyPainter will extract the piece of color (image) to be duplicated to generate the smudge effect
    KisPainter copyPainter(m_srcdev);
    copyPainter.setOpacity(opacity);
    
    // Normal algorithm: color is extracted as a rectangle, and then blitted with a mask.
    
    copyPainter.bitBlt(extractionTopLeft.x(), extractionTopLeft.y(), painter()->device(), x, y, sw, sh);
    copyPainter.end();
    painter()->bitBltWithFixedSelection(x, y, m_srcdev, maskDab, 0, 0, extractionTopLeft.x(), extractionTopLeft.y(), sw, sh);
    
    // Reverted algorithm: color is extracted with a mask, and then blitted as a rectangle.
    /*
    copyPainter.bitBltWithFixedSelection(extractionTopLeft.x(), extractionTopLeft.y(), painter()->device(), maskDab, 0, 0, x, y, sw, sh);
    copyPainter.end();
    painter()->bitBlt(x, y, m_srcdev, extractionTopLeft.x(), extractionTopLeft.y(), sw, sh);
    */
    
    // Stacked algorithm: both extraction and blitting are done with a mask.
    /*
    copyPainter.bitBltWithFixedSelection(extractionTopLeft.x(), extractionTopLeft.y(), painter()->device(), maskDab, 0, 0, x, y, sw, sh);
    copyPainter.end();
    painter()->bitBltWithFixedSelection(x, y, m_srcdev, maskDab, 0, 0, extractionTopLeft.x(), extractionTopLeft.y(), sw, sh);
    */
    
    return spacing(scale);
}
