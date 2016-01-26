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

#include "kis_experiment_paintop_settings.h"
#include "kis_current_outline_fetcher.h"

bool KisExperimentPaintOpSettings::paintIncremental()
{
    /**
     * The experiment brush supports working in the
     * WASH mode only!
     */
    return false;
}

QPainterPath KisExperimentPaintOpSettings::brushOutline(const KisPaintInformation &info, KisPaintOpSettings::OutlineMode mode) const
{
    QPainterPath path;
    if (mode == CursorIsOutline || mode == CursorIsCircleOutline || mode == CursorTiltOutline) {

        QRectF ellipse(0, 0, 3, 3);
        ellipse.translate(-ellipse.center());
        path.addEllipse(ellipse);


        ellipse.setRect(0,0, 12, 12);
        ellipse.translate(-ellipse.center());
        path.addEllipse(ellipse);

        QPainterPath tiltLine;
        QLineF tiltAngle(QPointF(0.0,0.0), QPointF(0.0,3.0));
        tiltAngle.setLength(50.0 * (1 - info.tiltElevation(info, 60.0, 60.0, true)));
        tiltAngle.setAngle((360.0 - fmod(KisPaintInformation::tiltDirection(info, true) * 360.0 + 270.0, 360.0))-3.0);
        tiltLine.moveTo(tiltAngle.p1());
        tiltLine.lineTo(tiltAngle.p2());
        tiltAngle.setAngle((360.0 - fmod(KisPaintInformation::tiltDirection(info, true) * 360.0 + 270.0, 360.0))+3.0);
        tiltLine.lineTo(tiltAngle.p2());
        tiltLine.lineTo(tiltAngle.p1());

        if (mode == CursorTiltOutline) {
            path.addPath(tiltLine);
        }

        path.translate(info.pos());
        
    }
    return path;
}
