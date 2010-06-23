/*
 *  Copyright (c) 2008,2009 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "kis_hatching_paintop.h"
#include "kis_hatching_paintop_settings.h"

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

KisHatchingPaintOp::KisHatchingPaintOp(const KisHatchingPaintOpSettings *settings, KisPainter * painter, KisImageWSP image)
        : KisBrushBasedPaintOp(settings, painter)
        , m_image(image)
{
    m_settings = new KisHatchingPaintOpSettings();
    settings->initializeTwin(m_settings);
    
    m_hatchingBrush = new HatchingBrush(m_settings);
    //m_opacityOption.readOptionSetting(settings);
    //m_opacityOption.sensor()->reset();
    //m_attributes.loadSettings(settings);
}

KisHatchingPaintOp::~KisHatchingPaintOp()
{
    delete m_hatchingBrush;
}

double KisHatchingPaintOp::paintAt(const KisPaintInformation& info)
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
    
    //DECLARING EMPTY pixel-only paint devices, note that those are smart pointers
    KisFixedPaintDeviceSP maskDab, hatchedDab = 0;
    
    //Declare a variable to store input-based scaling of the brush dab
    double scale = 1;   // TODO: use actual scale
    
    /* DEPENDS ON PRESSURE
    double scale = KisPaintOp::scaleForPressure(m_sizeOption.apply(info));
    */
    if ((scale * brush->width()) <= 0.01 || (scale * brush->height()) <= 0.01) return 1.0;
    
    
    //-----------POSITIONING code----------
    QPointF hotSpot = brush->hotSpot(scale, scale);
    QPointF pt = info.pos() - hotSpot;
    
    qint32 x, y;
    double xFraction, yFraction;
    
    splitCoordinate(pt.x(), &x, &xFraction);
    splitCoordinate(pt.y(), &y, &yFraction);
    
    if (!m_settings->subpixelprecision) {
        xFraction = 0;
        yFraction = 0;
    }
    //--------END POSITIONING CODE-----------
    
    /*--------VERY UGLY LOOKING IF-ELSE block, copypasted from SmudgeOp-------
    ---This IF-ELSE block is used to turn the mask created in the BrushTip dialogue
    into a beautiful SELECTION MASK (it's an opacity multiplier), intended to give
    the brush a "brush feel" (soft borders, round shape) despite it comes from a
    simple, ugly, hatched rectangle.
    The MASK is -----> maskDab
    The HATCHED part is -----> hatchedDab */
    if (brush->brushType() == IMAGE || brush->brushType() == PIPE_IMAGE) {
        maskDab = brush->paintDevice(device->colorSpace(), scale, 0.0, info, xFraction, yFraction);
        maskDab->convertTo(KoColorSpaceRegistry::instance()->alpha8());
    } else {
        maskDab = cachedDab();
        KoColor color = painter()->paintColor();
        color.convertTo(maskDab->colorSpace());
        brush->mask(maskDab, color, scale, scale, 0.0, info, xFraction, yFraction);
        maskDab->convertTo(KoColorSpaceRegistry::instance()->alpha8());
    }
    
    /*-----Convenient renaming for the limits of the maskDab, this will be used
    to hatch a dab of just the right size------*/
    qint32 sw = maskDab->bounds().width();
    qint32 sh = maskDab->bounds().height();
    
    //printf("maskWidth es %li y maskHeight es %li", brush->width(), brush->height());
    
    //quint8 origOpacity = m_opacityOption.apply(painter(), info);
    
    //------This If_block pre-fills the future hatchedDab with a pretty backgroundColor
    if (m_settings->opaquebackground) {
        KoColor aersh = painter()->backgroundColor();
        hatchedDab->fill(0, 0, (sw-1), (sh-1), aersh.data()); //this plus yellow background = french fry brush
    }
    
    /*-----This is the 2nd most important line, it's the line that creates the hatching
    but it doesn't paint anything visible in the user screen----------*/
    m_hatchingBrush->paint(hatchedDab, x, y, sw, sh, painter()->paintColor());
    
    //------THIS IS THE MOST IMPORTANT LINE, IT'S THE LINE THAT ACTUALLY PAINTS-------
    painter()->bitBlt(x, y, hatchedDab, maskDab, 0, 0, sw, sh);
    
    //printf ("Ancho: %i . Alto: %i. \n", limits.width(), limits.height());
    //painter()->setOpacity(origOpacity);
    
    /*-----It took me very long to realize the importance of this line, this is
    the line that makes all brushes be slow, even if they're small, yay!-------*/
    //TODO: change default scale setting to have a faster brush.
    return spacing(scale);
}
