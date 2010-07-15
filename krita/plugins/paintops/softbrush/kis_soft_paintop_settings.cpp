/*
 *  Copyright (c) 2009,2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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
#include "kis_soft_paintop_settings.h"
#include "kis_softop_option.h"
#include "kis_brush_size_option.h"
#include <kis_paint_action_type_option.h>
#include <kis_airbrush_option.h>


bool KisSoftPaintOpSettings::paintIncremental()
{
    return (enumPaintActionType)getInt("PaintOpAction", WASH) == BUILDUP;
}

bool KisSoftPaintOpSettings::isAirbrushing() const
{
    return getBool(AIRBRUSH_ENABLED);
}

int KisSoftPaintOpSettings::rate() const
{
    return getInt(AIRBRUSH_RATE);
}

void KisSoftPaintOpSettings::paintOutline ( const QPointF& pos, KisImageWSP image, QPainter& painter, KisPaintOpSettings::OutlineMode _mode ) const
{
    if (_mode != CURSOR_IS_OUTLINE) return;
    qreal width = getDouble(BRUSH_DIAMETER)  * getDouble(BRUSH_SCALE);
    qreal height = getDouble(BRUSH_DIAMETER) * getDouble(BRUSH_ASPECT)  * getDouble(BRUSH_SCALE);

    QRectF brush(0,0,width,height);
    brush.translate(-brush.center());
    painter.save();
    painter.translate( pos);
    painter.rotate( -getDouble(BRUSH_ROTATION));
    painter.setPen(Qt::black);
    painter.drawEllipse(image->pixelToDocument(brush));
    painter.restore();
}


QRectF KisSoftPaintOpSettings::paintOutlineRect ( const QPointF& pos, KisImageWSP image, KisPaintOpSettings::OutlineMode _mode ) const
{
    if (_mode != CURSOR_IS_OUTLINE) return QRectF();
    qreal width = getDouble(BRUSH_DIAMETER)  * getDouble(BRUSH_SCALE);
    qreal height = getDouble(BRUSH_DIAMETER) * getDouble(BRUSH_ASPECT)  * getDouble(BRUSH_SCALE);
    QRectF brush(0,0,width,height);
    brush.translate(-brush.center());
    QTransform m;
    m.reset();
    m.rotate( -getDouble(BRUSH_ROTATION) );
    brush = m.mapRect(brush);
    brush.adjust(-1,-1,1,1);
    return image->pixelToDocument(brush).translated(pos);
}

QPainterPath KisSoftPaintOpSettings::brushOutline() const
{
    qreal width = getInt(BRUSH_DIAMETER) * getDouble(BRUSH_SCALE);
    qreal height = getInt(BRUSH_DIAMETER) * getDouble(BRUSH_ASPECT) * getDouble(BRUSH_SCALE);
    QRectF brush(0,0,width,height);
    brush.translate(-brush.center());

    QPainterPath path;
    path.addEllipse(brush);
    
    QTransform m;
    m.reset();
    m.rotate( getDouble(BRUSH_ROTATION) );
    path = m.map(path);
    return path;
}


