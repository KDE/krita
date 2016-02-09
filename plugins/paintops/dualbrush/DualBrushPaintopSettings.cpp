/*
 *  Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
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

#include "DualBrushPaintopSettings.h"
#include "DualBrushOption.h"
#include <kis_paint_action_type_option.h>

DualBrushPaintOpSettings::DualBrushPaintOpSettings()
{
    qDebug() << "Creating DualBrushPaintOpSettingsWidget";
}

bool DualBrushPaintOpSettings::paintIncremental()
{
    return (enumPaintActionType)getInt("PaintOpAction", WASH) == BUILDUP;
}

bool DualBrushPaintOpSettings::isAirbrushing() const
{
    // return true if one of the constituent brushes is an airbrush
    return false;
}

int DualBrushPaintOpSettings::rate() const
{
    return 100;
}

QPainterPath DualBrushPaintOpSettings::brushOutline(const KisPaintInformation &info, OutlineMode mode) const
{
    QPainterPath path;
    if (mode == CursorIsOutline || mode == CursorIsCircleOutline || mode == CursorTiltOutline) {
//        qreal size = getInt(DUALBRUSH_RADIUS) * 2 + 1;
//        path = ellipseOutline(size, size, 1.0, 0.0);

//        QPainterPath tiltLine;
//        QLineF tiltAngle(QPointF(0.0,0.0), QPointF(0.0,size));
//        tiltAngle.setLength(qMax(size*qreal(0.5), qreal(50.0)) * (1 - info.tiltElevation(info, 60.0, 60.0, true)));
//        tiltAngle.setAngle((360.0 - fmod(KisPaintInformation::tiltDirection(info, true) * 360.0 + 270.0, 360.0))-3.0);
//        tiltLine.moveTo(tiltAngle.p1());
//        tiltLine.lineTo(tiltAngle.p2());
//        tiltAngle.setAngle((360.0 - fmod(KisPaintInformation::tiltDirection(info, true) * 360.0 + 270.0, 360.0))+3.0);
//        tiltLine.lineTo(tiltAngle.p2());
//        tiltLine.lineTo(tiltAngle.p1());

//        if (mode == CursorTiltOutline) {
//            path.addPath(tiltLine);
//        }

//        path.translate(info.pos());
    }
    return path;
}
