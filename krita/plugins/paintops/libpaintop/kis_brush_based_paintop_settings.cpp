/*
 *  Copyright (c) 2010 Sven Langkamp <sven.langkamp@gmail.com>
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

#include "kis_brush_based_paintop_settings.h"

#include <kis_paint_action_type_option.h>
#include <kis_airbrush_option.h>
#include "kis_brush_based_paintop_options_widget.h"
#include <kis_boundary.h>

bool KisBrushBasedPaintOpSettings::paintIncremental()
{
    if(hasProperty("PaintOpAction")) {
        return (enumPaintActionType)getInt("PaintOpAction", WASH) == BUILDUP;
    }
    return true;
}

bool KisBrushBasedPaintOpSettings::isAirbrushing() const
{
    return getBool(AIRBRUSH_ENABLED);
}


int KisBrushBasedPaintOpSettings::rate() const
{
    return getInt(AIRBRUSH_RATE);
}


QRectF KisBrushBasedPaintOpSettings::paintOutlineRect(const QPointF& pos, KisImageWSP image, KisPaintOpSettings::OutlineMode _mode) const
{
    KisBrushBasedPaintopOptionWidget* options = dynamic_cast<KisBrushBasedPaintopOptionWidget*>(optionsWidget());
    if(!options)
        return QRectF();
    
    if (_mode != CursorIsOutline) return QRectF();
    KisBrushSP brush = options->brush();
    QPointF hotSpot = brush->hotSpot(1.0, 1.0);
    QRectF rect = image->pixelToDocument(
                  QRectF(0, 0, brush->width() + 1, brush->height() + 1).translated(-(hotSpot + QPointF(0.5, 0.5)))
                  ).translated(pos);
    rect.adjust(-2, -2, 2, 2);
    return rect;
}

void KisBrushBasedPaintOpSettings::paintOutline(const QPointF& pos, KisImageWSP image, QPainter& painter, KisPaintOpSettings::OutlineMode _mode) const
{
    KisBrushBasedPaintopOptionWidget* options = dynamic_cast<KisBrushBasedPaintopOptionWidget*>(optionsWidget());
    if(!options)
        return;
    
    if (_mode != CursorIsOutline) return;
    KisBrushSP brush = options->brush();
    QPointF hotSpot = brush->hotSpot(1.0, 1.0);
    painter.setPen(Qt::black);
    painter.setBackground(Qt::black);
    painter.translate(pos - image->pixelToDocument(hotSpot + QPointF(0.5, 0.5)));
    painter.scale(1/image->xRes(), 1/image->yRes());
    brush->boundary()->paint(painter);
}

QPainterPath KisBrushBasedPaintOpSettings::brushOutline(const QPointF& pos,OutlineMode mode) const
{
    QPainterPath path;
    if (mode == CursorIsOutline) {
    
        KisBrushBasedPaintopOptionWidget* options = dynamic_cast<KisBrushBasedPaintopOptionWidget*>(optionsWidget());
        if(!options) {
            return KisPaintOpSettings::brushOutline(pos,mode);
        }
        
        KisBrushSP brush = options->brush();
        QPointF hotSpot = brush->hotSpot(1.0, 1.0);

        QTransform m;
        m.scale(brush->scale(), brush->scale());
        
        path = brush->boundary()->boundary();
        path = m.map(path);
        path.translate(-hotSpot);
        path.translate(pos);
    }
    return path;
}

