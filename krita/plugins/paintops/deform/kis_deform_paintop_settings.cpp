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

KisDeformPaintOpSettings::KisDeformPaintOpSettings()
    : KisOutlineGenerationPolicy<KisPaintOpSettings>(KisCurrentOutlineFetcher::SIZE_OPTION |
            KisCurrentOutlineFetcher::ROTATION_OPTION)
{
}

bool KisDeformPaintOpSettings::paintIncremental()
{
    return true;
}

bool KisDeformPaintOpSettings::isAirbrushing() const
{
    // version 2.3
    if (hasProperty(AIRBRUSH_ENABLED)) {
        return getBool(AIRBRUSH_ENABLED);
    }
    else {
        return getBool(DEFORM_USE_MOVEMENT_PAINT);
    }
}

int KisDeformPaintOpSettings::rate() const
{
    if (hasProperty(AIRBRUSH_RATE)) {
        return getInt(AIRBRUSH_RATE);
    }
    else {
        return KisPaintOpSettings::rate();
    }
}

QPainterPath KisDeformPaintOpSettings::brushOutline(const KisPaintInformation &info, OutlineMode mode) const
{
    QPainterPath path;
    if (mode == CursorIsOutline || mode == CursorIsCircleOutline || mode == CursorTiltOutline) {
        qreal width = getInt(BRUSH_DIAMETER);
        qreal height = getInt(BRUSH_DIAMETER) * getDouble(BRUSH_ASPECT);
        path = ellipseOutline(width, height, getDouble(BRUSH_SCALE), getDouble(BRUSH_ROTATION));
        
        QPainterPath tiltLine;
        QLineF tiltAngle(QPointF(0.0,0.0), QPointF(0.0,width));
        tiltAngle.setLength(qMax(width*0.5, 50.0) * (1 - info.tiltElevation(info, 60.0, 60.0, true)));
        tiltAngle.setAngle((360.0 - fmod(KisPaintInformation::tiltDirection(info, true) * 360.0 + 270.0, 360.0))-2.0);
        tiltLine.moveTo(tiltAngle.p1());
        tiltLine.lineTo(tiltAngle.p2());
        tiltAngle.setAngle((360.0 - fmod(KisPaintInformation::tiltDirection(info, true) * 360.0 + 270.0, 360.0))+2.0);
        tiltLine.lineTo(tiltAngle.p2());
        tiltLine.lineTo(tiltAngle.p1());
        
        path = outlineFetcher()->fetchOutline(info, this, path);
        
        if (mode == CursorTiltOutline) {
            path.addPath(outlineFetcher()->fetchOutline(info, this, tiltLine, 1.0, 0.0, true, 0, 0));
        }
    }
    return path;
}
