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

#include <kis_deform_paintop_settings.h>
#include <kis_deform_paintop_settings_widget.h>

#include <kis_brush_size_option.h>

#include <kis_airbrush_option.h>
#include <kis_deform_option.h>

bool KisDeformPaintOpSettings::paintIncremental()
{
    return true;
}

bool KisDeformPaintOpSettings::isAirbrushing() const
{
    // version 2.3
    if (hasProperty(AIRBRUSH_ENABLED)){
        return getBool(AIRBRUSH_ENABLED);
    }else{
        return getBool(DEFORM_USE_MOVEMENT_PAINT);
    }
}

int KisDeformPaintOpSettings::rate() const
{
    if (hasProperty(AIRBRUSH_RATE)){
        return getInt(AIRBRUSH_RATE);
    }else{
        return KisPaintOpSettings::rate();
    }
}

QPainterPath KisDeformPaintOpSettings::brushOutline(const QPointF& pos, KisPaintOpSettings::OutlineMode mode, qreal scale, qreal rotation) const
{
    QPainterPath path;
    if (mode == CursorIsOutline){
        qreal width = getInt(BRUSH_DIAMETER);
        qreal height = getInt(BRUSH_DIAMETER) * getDouble(BRUSH_ASPECT);
        path = ellipseOutline(width, height,getDouble(BRUSH_SCALE),getDouble(BRUSH_ROTATION) );
        QTransform m; m.reset(); m.scale(scale,scale); m.rotateRadians(rotation);
        path = m.map(path);
        path.translate(pos);
    }
    return path;
}
