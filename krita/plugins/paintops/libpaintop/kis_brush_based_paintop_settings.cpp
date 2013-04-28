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

bool KisBrushBasedPaintOpSettings::paintIncremental()
{
    if(hasProperty("PaintOpAction")) {
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

QPainterPath KisBrushBasedPaintOpSettings::brushOutline(const QPointF& pos, KisPaintOpSettings::OutlineMode mode, qreal scale, qreal rotation) const
{
    QPainterPath path;
    if (mode == CursorIsOutline) {
    
        KisBrushBasedPaintopOptionWidget* options = dynamic_cast<KisBrushBasedPaintopOptionWidget*>(optionsWidget());
        if(!options) {
            return KisPaintOpSettings::brushOutline(pos,mode);
        }
        
        KisBrushSP brush = options->brush();
        QPointF hotSpot = brush->hotSpot(1.0/brush->scale(),1.0/brush->scale(), -brush->angle(), KisPaintInformation());

        QTransform m;
        m.reset();
        m.rotateRadians(-rotation - brush->angle()); 
        m.scale(brush->scale() * scale, brush->scale() * scale);
        m.translate(-hotSpot.x(), -hotSpot.y());
        
        
        path = brush->outline();
        path = m.map(path);
        
        path.translate(pos);
    }
    return path;
}

bool KisBrushBasedPaintOpSettings::isValid()
{
    QString filename = getString("requiredBrushFile","");
    if (!filename.isEmpty()) {
        KisBrushSP brush = KisBrushServer::instance()->brushServer()->getResourceByFilename(filename);
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
