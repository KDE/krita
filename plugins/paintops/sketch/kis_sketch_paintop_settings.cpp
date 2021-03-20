/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_sketch_paintop_settings.h"

#include <kis_sketchop_option.h>

#include <kis_paint_action_type_option.h>
#include "kis_current_outline_fetcher.h"


KisSketchPaintOpSettings::KisSketchPaintOpSettings(KisResourcesInterfaceSP resourcesInterface)
    : KisBrushBasedPaintOpSettings(resourcesInterface)
{
}

bool KisSketchPaintOpSettings::paintIncremental()
{
    return (enumPaintActionType)getInt("PaintOpAction", WASH) == BUILDUP;
}

QPainterPath KisSketchPaintOpSettings::brushOutline(const KisPaintInformation &info, const OutlineMode &mode, qreal alignForZoom)
{
    bool isSimpleMode = getBool(SKETCH_USE_SIMPLE_MODE);

    if (!isSimpleMode) {
        return KisBrushBasedPaintOpSettings::brushOutline(info, mode, alignForZoom);
    }

    QPainterPath path;

    KisBrushSP brush = this->brush();

    if (brush && mode.isVisible) {
        // just circle supported
        qreal diameter = qMax(brush->width(), brush->height());
        path = ellipseOutline(diameter, diameter, 1.0, 0.0);

        path = outlineFetcher()->fetchOutline(info, this, path, mode, alignForZoom);
        if (mode.showTiltDecoration) {
            QPainterPath tiltLine =
                makeTiltIndicator(info, path.boundingRect().center(), diameter * 0.5, 3.0);
            path.addPath(outlineFetcher()->fetchOutline(info, this, tiltLine, mode, alignForZoom, 1.0, 0.0, true, path.boundingRect().center().x(), path.boundingRect().center().y()));
        }
    }
    return path;
}

bool KisSketchPaintOpSettings::hasPatternSettings() const
{
    return false;
}