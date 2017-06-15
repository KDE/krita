/*
 *  Copyright (c) 2008 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "kis_chalk_paintop_settings.h"

#include <kis_chalkop_option.h>

#include <kis_paint_action_type_option.h>
#include <kis_airbrush_option.h>

KisChalkPaintOpSettings::KisChalkPaintOpSettings()
{
}

bool KisChalkPaintOpSettings::paintIncremental()
{
    return (enumPaintActionType)getInt("PaintOpAction", WASH) == BUILDUP;
}

QPainterPath KisChalkPaintOpSettings::brushOutline(const KisPaintInformation &info, OutlineMode mode)
{
    QPainterPath path;
    if (mode == CursorIsOutline || mode == CursorIsCircleOutline || mode == CursorTiltOutline) {
        qreal size = getInt(CHALK_RADIUS) * 2 + 1;
        path = ellipseOutline(size, size, 1.0, 0.0);

        if (mode == CursorTiltOutline) {
            path.addPath(makeTiltIndicator(info, QPointF(0.0, 0.0), size * 0.5, 3.0));
        }

        path.translate(info.pos());
    }
    return path;
}

void KisChalkPaintOpSettings::setPaintOpSize(qreal value)
{
    ChalkProperties properties;
    properties.readOptionSetting(this);
    properties.radius = qRound(0.5 * value);
    properties.writeOptionSetting(this);
}

qreal KisChalkPaintOpSettings::paintOpSize() const
{
    ChalkProperties properties;
    properties.readOptionSetting(this);
    return properties.radius * 2;
}
