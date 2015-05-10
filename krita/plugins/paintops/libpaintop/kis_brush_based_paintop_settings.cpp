/*
 *  Copyright (c) 2010 Sven Langkamp <sven.langkamp@gmail.com>
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

#include "kis_brush_based_paintop_settings.h"

#include <kis_paint_action_type_option.h>
#include <kis_airbrush_option.h>
#include "kis_brush_based_paintop_options_widget.h"
#include <kis_boundary.h>
#include "kis_brush_server.h"


KisBrushBasedPaintOpSettings::KisBrushBasedPaintOpSettings()
    : KisOutlineGenerationPolicy<KisPaintOpSettings>(KisCurrentOutlineFetcher::SIZE_OPTION |
            KisCurrentOutlineFetcher::ROTATION_OPTION |
            KisCurrentOutlineFetcher::MIRROR_OPTION)
{
}

bool KisBrushBasedPaintOpSettings::paintIncremental()
{
    if (hasProperty("PaintOpAction")) {
        return (enumPaintActionType)getInt("PaintOpAction", WASH) == BUILDUP;
    }
    return true;
}

bool KisBrushBasedPaintOpSettings::isAirbrushing() const
{
    return getBool(AIRBRUSH_ENABLED);
}


int KisBrushBasedPaintOpSettings::rate() const
{
    return getInt(AIRBRUSH_RATE);
}

QPainterPath KisBrushBasedPaintOpSettings::brushOutlineImpl(const KisPaintInformation &info, OutlineMode mode, qreal additionalScale) const
{
    QPainterPath path;

    if (mode == CursorIsOutline || mode == CursorIsCircleOutline) {
        KisBrushBasedPaintopOptionWidget *widget = dynamic_cast<KisBrushBasedPaintopOptionWidget*>(optionsWidget());

        if (!widget) {
            return KisPaintOpSettings::brushOutline(info, mode);
        }


        KisBrushSP brush = widget->brush();
        qreal finalScale = brush->scale() * additionalScale;

        QPainterPath realOutline = brush->outline();

        if (mode == CursorIsCircleOutline) {
            QPainterPath ellipse;
            ellipse.addEllipse(realOutline.boundingRect());
            realOutline = ellipse;
        }

        path = outlineFetcher()->fetchOutline(info, this, realOutline, finalScale, brush->angle());
    }

    return path;
}

QPainterPath KisBrushBasedPaintOpSettings::brushOutline(const KisPaintInformation &info, OutlineMode mode) const
{
    return brushOutlineImpl(info, mode, 1.0);
}

bool KisBrushBasedPaintOpSettings::isValid() const
{
    QString filename = getString("requiredBrushFile", "");
    if (!filename.isEmpty()) {
        KisBrushSP brush = KisBrushServer::instance()->brushServer()->resourceByFilename(filename);
        if (!brush) {
            return false;
        }
    }
    return true;
}

bool KisBrushBasedPaintOpSettings::isLoadable()
{
    return (KisBrushServer::instance()->brushServer()->resources().count() > 0);
}
