/*
 *  Copyright (c) 2010 José Luis Vergara <pentalis@gmail.com>
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

#include "kis_waterymix_paintop.h"
#include "kis_waterymix_paintop_settings.h"

#include <cmath>
#include <QRect>

#include <KoColor.h>
#include <KoColorSpace.h>

#include <kis_image.h>
#include <kis_debug.h>

#include <kis_global.h>
#include <kis_paint_device.h>
#include <kis_painter.h>
#include <kis_types.h>
#include <kis_paintop.h>
#include <kis_brush_based_paintop.h>
#include <kis_paint_information.h>

#include <kis_pressure_opacity_option.h>

#include <KoColorSpaceRegistry.h>
#include <kis_selection.h>

#include <kis_paintop_registry.h>

#include <../../extensions/impasto/kis_rgbu8_height_color_space.h>
#include <../../extensions/impasto/kis_rgbu8_height_color_space.cpp>
#include <kis_group_layer.h>

KisWateryMixPaintOp::KisWateryMixPaintOp(const KisWateryMixPaintOpSettings *settings, KisPainter * painter, KisImageWSP image)
                   : KisBrushBasedPaintOp(settings, painter)
                   , m_image(image)
{
    m_settings = new KisWateryMixPaintOpSettings();
    //settings->initializeTwin(m_settings);

    m_opacityOption.sensor()->reset();
    m_sizeOption.sensor()->reset();
    
    hardcodedSettings.absortionrate = 0.2;
    hardcodedSettings.humidity = 0;
}

KisWateryMixPaintOp::~KisWateryMixPaintOp()
{
}

qreal KisWateryMixPaintOp::paintAt(const KisPaintInformation& info)
{
    //------START SIMPLE ERROR CATCHING-------
    if (!painter()->device()) return 1;
    
    //Simple convenience renaming, I'm thinking of removing these inherited quirks
    KisBrushSP brush = m_brush;
    KisPaintDeviceSP device = painter()->device();

    //Macro to catch errors
    Q_ASSERT(brush);

    //----------SIMPLE error catching code, maybe it's not even needed------
    if (!brush) return 1;
    if (!brush->canPaintFor(info)) return 1;

    double scale = KisPaintOp::scaleForPressure(m_sizeOption.apply(info));
    if ((scale * brush->width()) <= 0.01 || (scale * brush->height()) <= 0.01) return 1.0;

    quint8 origOpacity = m_opacityOption.apply(painter(), info);

    //-----------POSITIONING code----------
    QPointF hotSpot = brush->hotSpot(scale, scale);
    QPointF pt = info.pos() - hotSpot;

    qint32 x, y;
    qreal xFraction, yFraction;

    splitCoordinate(pt.x(), &x, &xFraction);
    splitCoordinate(pt.y(), &y, &yFraction);

    //--------END POSITIONING CODE-----------

    //DECLARING EMPTY pixel-only paint device, note that it is a smart pointer
    KisFixedPaintDeviceSP maskDab = 0;
    KisFixedPaintDeviceSP paintDab = 0;
    
    /*--------copypasted from SmudgeOp-------
    ---This IF-ELSE block is used to turn the mask created in the BrushTip dialogue
    into a beautiful SELECTION MASK (it's an opacity multiplier), intended to give
    the brush a "brush feel" (soft borders, round shape).*/
    if (brush->brushType() == IMAGE || brush->brushType() == PIPE_IMAGE) {
        maskDab = brush->paintDevice(device->colorSpace(), scale, 0.0, info, xFraction, yFraction);
        maskDab->convertTo(KoColorSpaceRegistry::instance()->alpha8());
        
        paintDab = brush->paintDevice(device->colorSpace(), scale, 0.0, info, xFraction, yFraction);
    } else {
        maskDab = cachedDab();
        KoColor color = painter()->paintColor();
        color.convertTo(maskDab->colorSpace());
        brush->mask(maskDab, color, scale, scale, 0.0, info, xFraction, yFraction);
        maskDab->convertTo(KoColorSpaceRegistry::instance()->alpha8());
        
        paintDab = cachedDab();
        brush->mask(paintDab, color, scale, scale, 0.0, info, xFraction, yFraction);
    }

    /*-----Convenient renaming for the limits of the maskDab, this will be used
    to fill a dab of just the right size------*/
    qint32 sw = maskDab->bounds().width();
    qint32 sh = maskDab->bounds().height();
    
    /* ------- algorithm  ------ */
    
    /*
    What this paintop ought to do (and currently doesn't):
    --In the first pass, the brush is empty, devoid of color.
    Its contents will be called dilutedDab.
    --For each pass (including the first), do:
    ----Take a sample of the image beneath the brush respecting
    the transparency of the brush mask (this simulates paint
    transfer from the canvas to the brush, the brush mask is
    considered equivalent to the brush hairs in real life).
    Call this sample the absorbedDab.
    ----Blit dilutedDab on the canvas.
    ----Mix (blit) absorbedDab into dilutedDab.
    ----Dilute diluteDab by using an operation equivalent to a
    blur filter whose blurring is either a fixed distance or a 
    fraction of the dab size.
    TODO: implement this algorithm in fast, non-crashy code.
    Use HardcodedSettings on the meanwhile.
    */
    
    // Absorb
    /*
    // This theorethically clones the subyacent area
    KisPaintDeviceSP clone = new KisPaintDevice(painter()->device()->colorSpace());
    KisPainter copyPainter(clone);
    quint8 opacity = OPACITY_OPAQUE_U8;
    //opacity *= hardcodedSettings.absortionrate;
    copyPainter.setOpacity(opacity);
    copyPainter.bitBlt(0, 0, device, pt.x(), pt.y(), sw, sh);
    qDebug() << clone->exactBounds();
    
    // This theorethically mixes the stuff
    KisPaintDeviceSP blotch = new KisPaintDevice(painter()->device()->colorSpace());
    KisPainter maskPainter(blotch);
    maskPainter.bltFixed(0, 0, paintDab, 0, 0, sw, sh);
    maskPainter.bitBlt(0, 0, clone, 0, 0, sw, sh);
    //maskPainter.bitBltFixedSelection(0, 0, clone, maskDab, 0, 0, sw, sh);
    qDebug() << blotch->exactBounds();
    */
    // Dilute
  
    // Blit the results to the screen.blotch
    painter()->bltFixed(x, y, paintDab, 0, 0, sw, sh);
    //painter()->setOpacity(origOpacity);

    /*-----It took me very long to realize the importance of this line, this is
    the line that makes all brushes be slow, even if they're small, yay!-------*/
    return spacing(scale);
}

/*
void KisWateryMixPaintOp::absorb ()
{
}


void KisWateryMixPaintOp::dilute ()
{
}
*/
