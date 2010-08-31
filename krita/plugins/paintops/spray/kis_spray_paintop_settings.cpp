/*
 *  Copyright (c) 2008,2009,2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include <cmath>

#include <kis_paint_action_type_option.h>
#include <kis_color_option.h>

#include "kis_spray_paintop_settings.h"
#include "kis_sprayop_option.h"
#include "kis_spray_shape_option.h"
#include <kis_airbrush_option.h>

bool KisSprayPaintOpSettings::paintIncremental()
{
    return (enumPaintActionType)getInt("PaintOpAction", WASH) == BUILDUP;
}

bool KisSprayPaintOpSettings::isAirbrushing() const
{
    return getBool(AIRBRUSH_ENABLED);
}

int KisSprayPaintOpSettings::rate() const
{
    return getInt(AIRBRUSH_RATE);
}


QPainterPath KisSprayPaintOpSettings::brushOutline(const QPointF& pos, KisPaintOpSettings::OutlineMode mode, qreal scale, qreal rotation) const
{
    Q_UNUSED(pos);
    QPainterPath path;
    if (mode == CursorIsOutline){
        qreal width = getInt(SPRAY_DIAMETER);
        qreal height = getInt(SPRAY_DIAMETER) * getDouble(SPRAY_ASPECT);
        path = ellipseOutline(width, height, getDouble(SPRAY_SCALE) * scale , getDouble(SPRAY_ROTATION) - rotation*180.0/M_PI);
        path.translate(pos);
    }
    return path;
}
