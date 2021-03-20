/*
 *  SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2010-2011 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <compositeops/KoVcMultiArchBuildSupport.h> //MSVC requires that Vc come first
#include "kis_auto_brush_factory.h"

#include <QDomDocument>

#include "kis_auto_brush.h"
#include "kis_mask_generator.h"
#include <kis_dom_utils.h>

KisBrushSP KisAutoBrushFactory::createBrush(const QDomElement &brushDefinition, KisResourcesInterfaceSP resourcesInterface)
{
    Q_UNUSED(resourcesInterface);

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
