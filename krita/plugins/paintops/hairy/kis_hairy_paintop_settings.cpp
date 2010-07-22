/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
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

#include <QPainter>

#include "kis_image.h"

#include "kis_hairy_paintop_settings.h"
#include "kis_hairy_bristle_option.h"
#include "kis_hairy_shape_option.h"
#include "kis_brush_based_paintop_options_widget.h"
#include "kis_boundary.h"

void KisHairyPaintOpSettings::paintOutline(const QPointF& pos, KisImageWSP image, QPainter& painter, KisPaintOpSettings::OutlineMode _mode) const
{
    double scale = getDouble(HAIRY_BRISTLE_SCALE);

    KisBrushBasedPaintopOptionWidget* options = dynamic_cast<KisBrushBasedPaintopOptionWidget*>(optionsWidget());
    if(!options)
        return;
    
    if (_mode != CursorIsOutline) return;
    KisBrushSP brush = options->brush();
    QPointF hotSpot = brush->hotSpot(1.0, 1.0);
    painter.setPen(Qt::black);
    painter.setBackground(Qt::black);
        
    painter.translate(paintOutlineRect(pos, image, _mode).topLeft());
    painter.scale(1/image->xRes()*scale, 1/image->yRes()*scale);
    brush->boundary()->paint(painter);
    painter.restore();
 }
 
 
QRectF KisHairyPaintOpSettings::paintOutlineRect(const QPointF& pos, KisImageWSP image, KisPaintOpSettings::OutlineMode _mode) const
{
    KisBrushBasedPaintopOptionWidget* options = dynamic_cast<KisBrushBasedPaintopOptionWidget*>(optionsWidget());
    if(!options)
        return QRectF();
    
    if (_mode != CursorIsOutline) return QRectF();
    KisBrushSP brush = options->brush();
    QPointF hotSpot = brush->hotSpot(1.0, 1.0);

    double scale = getDouble(HAIRY_BRISTLE_SCALE);
    QTransform m;
    m.reset();
    m.scale(scale, scale);   
    
    QRectF rect = QRectF(0, 0, brush->width(), brush->height()).translated(-(hotSpot + QPointF(0.5, 0.5)));
    rect = image->pixelToDocument(m.mapRect(rect)).translated(pos);
    return rect;
}

QPainterPath KisHairyPaintOpSettings::brushOutline(const QPointF& pos,OutlineMode mode) const
{
    QPainterPath path;
    if (mode == CursorIsOutline){
        path = KisBrushBasedPaintOpSettings::brushOutline(QPointF(0.0,0.0),mode);
        double scale = getDouble(HAIRY_BRISTLE_SCALE);
        QTransform m;
        m.reset();
        m.scale(scale, scale);
        path = m.map(path);
        path.translate(pos);
    }
    return path;
}

