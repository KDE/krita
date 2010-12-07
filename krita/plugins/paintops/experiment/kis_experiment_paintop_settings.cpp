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

#include <kis_paint_action_type_option.h>

#include <QPainter>

#include "kis_experiment_paintop_settings.h"
#include "kis_experimentop_option.h"
#include "kis_experiment_shape_option.h"
#include "kis_image.h"
#include <kis_paintop_settings_widget.h>

bool KisExperimentPaintOpSettings::paintIncremental()
{
    return (enumPaintActionType)getInt("PaintOpAction", WASH) == BUILDUP;
}

bool KisExperimentPaintOpSettings::mousePressEvent(const KisPaintInformation& info, Qt::KeyboardModifiers modifiers)
{
    if (modifiers == (Qt::ControlModifier | Qt::ShiftModifier)) {
        setProperty(MIRROR_X,info.pos().x());
        setProperty(MIRROR_Y,info.pos().y());
        return false;
    }
    return true;
}

QPainterPath KisExperimentPaintOpSettings::brushOutline(const QPointF& pos, KisPaintOpSettings::OutlineMode mode, qreal scale, qreal rotation) const
{
    QPainterPath path = KisPaintOpSettings::brushOutline(pos, mode, scale, rotation);
    
    QPointF mirrorPosition(getDouble(MIRROR_X),getDouble(MIRROR_Y));
    int mirrorLineSize = 50;
    
    if (getBool(EXPERIMENT_MIRROR_HORZ)){
        path.moveTo(mirrorPosition.x(), 0);
        path.lineTo(mirrorPosition.x(), mirrorLineSize);
    }
    
    if (getBool(EXPERIMENT_MIRROR_VERT)){
        path.moveTo(0,mirrorPosition.y());
        path.lineTo(mirrorLineSize,mirrorPosition.y());
    }
    
    return path;
}



