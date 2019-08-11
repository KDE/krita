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
#include "kis_current_outline_fetcher.h"


KisSketchPaintOpSettings::KisSketchPaintOpSettings()
{
}

bool KisSketchPaintOpSettings::paintIncremental()
{
    return (enumPaintActionType)getInt("PaintOpAction", WASH) == BUILDUP;
}

QPainterPath KisSketchPaintOpSettings::brushOutline(const KisPaintInformation &info, const OutlineMode &mode)
{
    bool isSimpleMode = getBool(SKETCH_USE_SIMPLE_MODE);

    if (!isSimpleMode) {
        return KisBrushBasedPaintOpSettings::brushOutline(info, mode);
    }

    QPainterPath path;

    KisBrushSP brush = this->brush();

    if (brush && mode.isVisible) {
        // just circle supported
        qreal diameter = qMax(brush->width(), brush->height());
        path = ellipseOutline(diameter, diameter, 1.0, 0.0);

        path = outlineFetcher()->fetchOutline(info, this, path, mode);
        if (mode.showTiltDecoration) {
            QPainterPath tiltLine =
                makeTiltIndicator(info, path.boundingRect().center(), diameter * 0.5, 3.0);
            path.addPath(outlineFetcher()->fetchOutline(info, this, tiltLine, mode, 1.0, 0.0, true, path.boundingRect().center().x(), path.boundingRect().center().y()));
        }
    }
    return path;
}

