/*
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "kis_sketch_paintop_settings.h"

#include <kis_sketchop_option.h>

#include <kis_paint_action_type_option.h>
#include <kis_airbrush_option.h>

KisSketchPaintOpSettings::KisSketchPaintOpSettings()
{
}

bool KisSketchPaintOpSettings::paintIncremental()
{
    return (enumPaintActionType)getInt("PaintOpAction", WASH) == BUILDUP;
}

bool KisSketchPaintOpSettings::isAirbrushing() const
{
    return getBool(AIRBRUSH_ENABLED);
}

int KisSketchPaintOpSettings::rate() const
{
    return getInt(AIRBRUSH_RATE);
}


void KisSketchPaintOpSettings::paintOutline(const QPointF& pos, KisImageWSP image, QPainter& painter, KisPaintOpSettings::OutlineMode _mode) const
{
    if (_mode != CURSOR_IS_OUTLINE) return;
    qreal size = getInt(SKETCH_RADIUS);
    painter.setPen(Qt::black);
    painter.drawEllipse(image->pixelToDocument(QRectF(0, 0, size, size).translated(- QPoint(size * 0.5, size * 0.5))).translated(pos));
}


QRectF KisSketchPaintOpSettings::paintOutlineRect(const QPointF& pos, KisImageWSP image, KisPaintOpSettings::OutlineMode _mode) const
{
    if (_mode != CURSOR_IS_OUTLINE) return QRectF();
    qreal size = getInt(SKETCH_RADIUS);
    size += 10;
    return image->pixelToDocument(QRectF(0, 0, size, size).translated(- QPoint(size * 0.5, size * 0.5))).translated(pos);
}

QPainterPath KisSketchPaintOpSettings::brushOutline(OutlineMode mode) const
{
    QPainterPath path;
    if (mode == CURSOR_IS_OUTLINE){
        qreal size = getInt(SKETCH_RADIUS) * 2 + 1;
    
        QRectF rc(0, 0, size, size);
        rc.translate(-rc.center());
    
        path.addEllipse(rc);
    }
    return path;
}


#if defined(HAVE_OPENGL)
QString KisSketchPaintOpSettings::modelName() const
{
    return "3d-pencil";
}
#endif

