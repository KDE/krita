/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_auto_brush_factory.h"

#include <QDomDocument>

#include "kis_auto_brush.h"
#include "kis_mask_generator.h"

KisBrushSP KisAutoBrushFactory::getOrCreateBrush(const QDomElement& brushDefinition)
{
    KisMaskGenerator* mask = KisMaskGenerator::fromXML(brushDefinition);
    double angle = brushDefinition.attribute("brush_angle", "0.0").toDouble();
    KisBrushSP brush = new KisAutoBrush(mask, angle);
    double spacing = brushDefinition.attribute("brush_spacing", "1.0").toDouble();
    brush->setSpacing(spacing);
    return brush;
}
