/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004-2008 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Clarence Dang <dang@kde.org>
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

// All pieces of color extracted from the canvas will be centered around ANCHOR_POINT
const QPoint ANCHOR_POINT = QPoint(0, 0);


KisSmudgeOp::KisSmudgeOp(const KisBrushBasedPaintOpSettings *settings, KisPainter *painter, KisImageWSP image)
        : KisBrushBasedPaintOp(settings, painter)
        , m_firstRun(true)
        , m_tempDev(0)
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

    m_tempDev = new KisPaintDevice(painter->device()->colorSpace());
    
    // Initializing to a valid value to avoid weird errors during modifications
    m_wholeTempData = QRect(0, 0, 0, 0);
    
    m_color = painter->paintColor();
}

KisSmudgeOp::~KisSmudgeOp()
{
}

/* To smudge, one does the following:

 1.- First step: initialize a temporary paint device (m_tempDev) with a copy of the colors below the mouse pointer.
 All other times:
 2.- Vanishing step: Reduce the transparency of the temporary paint device so as to let it mix gradually.
 3.- Combine: Combine the temporary device with the piece the brush currently is 'painting', according to a ratio:
 in this case, opacity. (This is what in the first step does the copying of the data).
 4.- Blit to screen: This combination is then composited upon the actual image.
 5.- Special case: If the size of the dab (brush mask) changes during the stroke (for example, when
 using a stylus sensitive to pressure), align the colors extracted to the center of the previously absorbed colors,
 and in the vanishing step, ensure that all the colors have their opacity slowly reduced, not just the ones below
 the current brush mask.
 
 For the sake of speed optimization, the extent of the largest area of color contained in the
 temporary device is cached such that only the colored areas are considered.
 TODO: Make this cached value dump colors that have faded nearly completely and lie outside of the rectangle (dab)
 of the current iteration.
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
    
    /* Align a point that represents the top-left corner of the brush-stroke-rendering
    with the mouse pointer and take into account the brush mask size */
    QPointF hotSpot = brush->hotSpot(scale, scale);
    QPointF pt = info.pos() - hotSpot;

    /* Split the coordinates into integer plus fractional parts. The integer
    is where the dab will be positioned and the fractional part determines
    the sub-pixel positioning. */
    qint32 x, y;
    double xFraction, yFraction;

    splitCoordinate(pt.x(), &x, &xFraction);
    splitCoordinate(pt.y(), &y, &yFraction);

    KisFixedPaintDeviceSP maskDab = 0;

    // Extract the brush mask (maskDab) from brush, and turn it into a transparency mask (alpha8).
    if (brush->brushType() == IMAGE || brush->brushType() == PIPE_IMAGE) {
        // This is for bitmap brushes
        maskDab = brush->paintDevice(painter()->device()->colorSpace(), scale, 0.0, info, xFraction, yFraction);
        maskDab->convertTo(KoColorSpaceRegistry::instance()->alpha8());
    } else {
        // This is for parametric brushes, those created in the Autobrush popup config dialogue
        maskDab = cachedDab();
        brush->mask(maskDab, m_color, scale, scale, 0.0, info, xFraction, yFraction);
        maskDab->convertTo(KoColorSpaceRegistry::instance()->alpha8());
    }

    // Convenient renaming for the limits of the maskDab
    qint32 sw = maskDab->bounds().width();
    qint32 sh = maskDab->bounds().height();
    
    /* Prepare the top left corner of the temporary paint device where the extracted color will be drawn */
    QPoint extractionTopLeft = QPoint(ANCHOR_POINT.x() - sw / 2,
                                      ANCHOR_POINT.y() - sh / 2);
                                      
    /* In the block below, the opacity of the colors stored in m_tempDev
    is reduced in opacity. Nothing of the color present inside it is left out */
    quint8 opacity = OPACITY_OPAQUE_U8;
    if (!m_firstRun) {
        opacity = m_rateOption.apply(opacity, info);
        /* Without those limits, the smudge brush doesn't smudge anymore, it either makes a single
        dropplet of color, or drags a frame indefinitely over the canvas. */
        opacity = qBound(MIXABLE_LOWER_LIMIT, opacity, MIXABLE_UPPER_LIMIT);
        
        // Update the whole temporary data area, only grow it, don't shrink it. TODO: Shrink it when relevant
        QRect currentTempDataRect = QRect(extractionTopLeft, maskDab->bounds().size());
        if (currentTempDataRect.contains(m_wholeTempData)) {
            m_wholeTempData = currentTempDataRect;
        }
        
        // Reduce the opacity of all the data contained therein
        KisRectIterator it = m_tempDev->createRectIterator(m_wholeTempData.x(), m_wholeTempData.y(),
                                                           m_wholeTempData.width(), m_wholeTempData.height());
        KoColorSpace* cs = m_tempDev->colorSpace();
        while (!it.isDone()) {
            cs->setOpacity(it.rawData(), quint8(cs->opacityF(it.rawData()) * opacity), 1);
            ++it;
        }
        
        // Invert the opacity value for color absorption in the next lines (copyPainter)
        opacity = OPACITY_OPAQUE_U8 - opacity;
    }
    else {
        m_firstRun = false;
        m_wholeTempData = QRect(extractionTopLeft, maskDab->bounds().size());
    }
                                      
    /* copyPainter will extract the piece of color (image) to be duplicated to generate the smudge effect,
    it extracts a simple unmasked rectangle and adds it to what was extracted before in this same block of code,
    this sometimes shows artifacts when the brush is used with stylus and high spacing */
    KisPainter copyPainter(m_tempDev);
    copyPainter.setOpacity(opacity);
    copyPainter.bitBlt(extractionTopLeft.x(), extractionTopLeft.y(), painter()->device(), x, y, sw, sh);
    copyPainter.end();
    
    // This is the line that renders the extracted colors to the screen, with maskDab giving it the brush shape
    painter()->bitBltWithFixedSelection(x, y, m_tempDev, maskDab, 0, 0, extractionTopLeft.x(), extractionTopLeft.y(), sw, sh);
    
    return spacing(scale);
}
