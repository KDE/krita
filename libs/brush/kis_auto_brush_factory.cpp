/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2010-2011 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include <compositeops/KoVcMultiArchBuildSupport.h> //MSVC requires that Vc come first
#include "kis_auto_brush_factory.h"

#include <QDomDocument>

#include "kis_auto_brush.h"
#include "kis_mask_generator.h"
#include <kis_dom_utils.h>

KisBrushSP KisAutoBrushFactory::createBrush(const QDomElement &brushDefinition)
{
    KisMaskGenerator* mask = KisMaskGenerator::fromXML(brushDefinition.firstChildElement("MaskGenerator"));
    double angle = KisDomUtils::toDouble(brushDefinition.attribute("angle", "0.0"));
    double randomness = KisDomUtils::toDouble(brushDefinition.attribute("randomness", "0.0"));
    qreal density = KisDomUtils::toDouble(brushDefinition.attribute("density", "1.0"));
    double spacing = KisDomUtils::toDouble(brushDefinition.attribute("spacing", "1.0"));
    bool useAutoSpacing = KisDomUtils::toInt(brushDefinition.attribute("useAutoSpacing", "0"));
    qreal autoSpacingCoeff = KisDomUtils::toDouble(brushDefinition.attribute("autoSpacingCoeff", "1.0"));

    KisBrushSP brush = KisBrushSP(new KisAutoBrush(mask, angle, randomness, density));
    brush->setSpacing(spacing);
    brush->setAutoSpacing(useAutoSpacing, autoSpacingCoeff);
    return brush;
}
