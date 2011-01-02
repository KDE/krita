/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004-2008 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Clarence Dang <dang@kde.org>
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2004,2010 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  Copyright (c) 2010 José Luis Vergara Toloza <pentalis@gmail.com>
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

KisSmudgeOp::KisSmudgeOp(const KisBrushBasedPaintOpSettings *settings, KisPainter *painter, KisImageWSP image)
        : KisBrushBasedPaintOp(settings, painter), m_tempDev(0)
{
    Q_UNUSED(image);
    Q_ASSERT(settings);
    Q_ASSERT(painter);
    m_sizeOption.readOptionSetting(settings);
    m_rateOption.readOptionSetting(settings);
    m_compositeOption.readOptionSetting(settings);
    m_sizeOption.sensor()->reset();
    m_rateOption.sensor()->reset();
    m_compositeOption.sensor()->reset();

    m_tempDev  = new KisPaintDevice(painter->device()->colorSpace());
    m_firstRun = true;
}

KisSmudgeOp::~KisSmudgeOp()
{
}



qreal KisSmudgeOp::paintAt(const KisPaintInformation& info)
{
    // Simple error catching
    if (!painter()->device())
        return 1.0;
    
    KisBrushSP brush = m_brush;
    
    if(!brush || !brush->canPaintFor(info))
        return 1.0;
    
    // get the scaling factor calculated by the size option
    double scale = m_sizeOption.apply(info);
    
    // don't paint anything if the brush is too samll
    if((scale*brush->width()) <= 0.01 || (scale*brush->height()) <= 0.01)
        return 1.0;
    
    setCurrentScale(scale);
    
    QPointF point = info.pos() - brush->hotSpot(scale, scale);
    
    qint32 x, y;
    qreal xFraction, yFraction;
    splitCoordinate(point.x(), &x, &xFraction);
    splitCoordinate(point.y(), &y, &yFraction);
    
    KisFixedPaintDeviceSP maskDab = 0;
    
    // Extract the brush mask (maskDab) from brush with the correct scaled size
    if(brush->brushType() == IMAGE || brush->brushType() == PIPE_IMAGE) {
        // This is for bitmap brushes
        maskDab = brush->paintDevice(painter()->device()->colorSpace(), scale, 0.0, info, xFraction, yFraction);
    } else {
        // This is for parametric brushes, those created in the Autobrush popup config dialogue
        maskDab = cachedDab();
        brush->mask(maskDab, painter()->paintColor(), scale, scale, 0.0, info, xFraction, yFraction);
    }
    
    // transforms the fixed paint device with the current brush to alpha color space (to use it as alpha/transparency mask)
    maskDab->convertTo(KoColorSpaceRegistry::instance()->alpha8());
    
    // GET the opacy calculated by the rate option (apply is misleading because the opacy will not be applied)
    quint8 newOpacity = m_rateOption.apply(OPACITY_OPAQUE_U8, info);
    
    if(!m_firstRun) {
        // set opacity calculated by the rate option
        // then blit the temporary painting device on the canvas at the current brush position
        // the alpha mask (maskDab) will be used here to only blit the pixels that lie in the area (shape) of the brush
        painter()->setOpacity(newOpacity / 2);
        painter()->setCompositeOp(COMPOSITE_OVER);
        painter()->bitBltWithFixedSelection(x, y, m_tempDev, maskDab, maskDab->bounds().width(), maskDab->bounds().height());
        painter()->setOpacity(newOpacity);
        painter()->setCompositeOp(COMPOSITE_COPY_OPACITY);
        painter()->bitBltWithFixedSelection(x, y, m_tempDev, maskDab, maskDab->bounds().width(), maskDab->bounds().height());
    }
    else m_firstRun = false;
    
    // IMPORTANT: clear the temporary painting device to color black with zero opacity
    //            it will only clear the extents of the brush
    m_tempDev->clear(QRect(0, 0, brush->width(), brush->height()));
    
    KisPainter copyPainter(m_tempDev);
    
    // reset composite mode and opacity
    // then cut out the area from the canvas under the brush
    // and blit it to the temporary painting device
    copyPainter.setCompositeOp(COMPOSITE_OVER);
    copyPainter.setOpacity(OPACITY_OPAQUE_U8);
    copyPainter.bitBlt(0, 0, painter()->device(), x, y, brush->width(), brush->width());
    
    // if the user selected the color smudge option
    // we will mix some color into the temorary painting device (m_tempDev)
    if(m_compositeOption.isChecked()) {
        // this will apply the composite mode and the opacy (selected by the user)
        // to copyPainter
        m_compositeOption.apply(&copyPainter, OPACITY_OPAQUE_U8, info);
        
        // paint a rectangle with the current color (foreground color)
        // into the temporary painting device
        copyPainter.setFillStyle(KisPainter::FillStyleForegroundColor);
        copyPainter.setPaintColor(painter()->paintColor());
        copyPainter.paintRect(maskDab->bounds());
    }
    
    copyPainter.end();
    return spacing(scale);
}
