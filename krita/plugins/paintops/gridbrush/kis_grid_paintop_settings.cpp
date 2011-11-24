/*
 * Copyright (c) 2009,2010 Lukáš Tvrdý (lukast.dev@gmail.com)
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

#include <kis_paint_action_type_option.h>

#include "kis_grid_paintop_settings.h"
#include "kis_grid_paintop_settings_widget.h"

#include "kis_gridop_option.h"
#include "kis_grid_shape_option.h"
#include <kis_color_option.h>


bool KisGridPaintOpSettings::paintIncremental()
{
    return (enumPaintActionType)getInt("PaintOpAction", WASH) == BUILDUP;
}

QPainterPath KisGridPaintOpSettings::brushOutline(const QPointF& pos, KisPaintOpSettings::OutlineMode mode, qreal scale, qreal rotation) const
{
    QPainterPath path;
    if (mode == CursorIsOutline) {
        qreal sizex = getInt(GRID_WIDTH) * getDouble(GRID_SCALE) * scale;
        qreal sizey = getInt(GRID_HEIGHT) * getDouble(GRID_SCALE) * scale;
        QRectF rc(0, 0, sizex, sizey);
        rc.translate(-rc.center());
        QTransform m;
        m.reset();
        m.rotate(rotation);
        path = m.map(path);
        path.addRect(rc);
        path.translate(pos);
    }
    return path;
}
