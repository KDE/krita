/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisHairyBristleOptionData.h"

#include "kis_properties_configuration.h"
#include <kis_paintop_lod_limitations.h>


const QString HAIRY_BRISTLE_USE_MOUSEPRESSURE = "HairyBristle/useMousePressure";
const QString HAIRY_BRISTLE_SCALE = "HairyBristle/scale";
const QString HAIRY_BRISTLE_SHEAR = "HairyBristle/shear";
const QString HAIRY_BRISTLE_RANDOM = "HairyBristle/random";
const QString HAIRY_BRISTLE_DENSITY = "HairyBristle/density";
const QString HAIRY_BRISTLE_THRESHOLD = "HairyBristle/threshold";
const QString HAIRY_BRISTLE_ANTI_ALIASING = "HairyBristle/antialias";
const QString HAIRY_BRISTLE_USE_COMPOSITING = "HairyBristle/useCompositing";
const QString HAIRY_BRISTLE_CONNECTED = "HairyBristle/isConnected";


bool KisHairyBristleOptionData::read(const KisPropertiesConfiguration *setting)
{
    useMousePressure = setting->getBool(HAIRY_BRISTLE_USE_MOUSEPRESSURE, false);
    shearFactor = setting->getDouble(HAIRY_BRISTLE_SHEAR, 0.0);
    randomFactor = setting->getDouble(HAIRY_BRISTLE_RANDOM, 2.0);
    scaleFactor = setting->getDouble(HAIRY_BRISTLE_SCALE, 2.0);
    densityFactor = setting->getDouble(HAIRY_BRISTLE_DENSITY, 100.0);
    threshold = setting->getBool(HAIRY_BRISTLE_THRESHOLD, false);
    antialias = setting->getBool(HAIRY_BRISTLE_ANTI_ALIASING, false);
    useCompositing = setting->getBool(HAIRY_BRISTLE_USE_COMPOSITING, false);
    connectedPath = setting->getBool(HAIRY_BRISTLE_CONNECTED, false);

    return true;
}

void KisHairyBristleOptionData::write(KisPropertiesConfiguration *setting) const
{
    setting->setProperty(HAIRY_BRISTLE_USE_MOUSEPRESSURE, useMousePressure);
    setting->setProperty(HAIRY_BRISTLE_SHEAR, shearFactor);
    setting->setProperty(HAIRY_BRISTLE_RANDOM, randomFactor);
    setting->setProperty(HAIRY_BRISTLE_SCALE, scaleFactor);
    setting->setProperty(HAIRY_BRISTLE_DENSITY, densityFactor);
    setting->setProperty(HAIRY_BRISTLE_THRESHOLD, threshold);
    setting->setProperty(HAIRY_BRISTLE_ANTI_ALIASING, antialias);
    setting->setProperty(HAIRY_BRISTLE_USE_COMPOSITING, useCompositing);
    setting->setProperty(HAIRY_BRISTLE_CONNECTED, connectedPath);
}

KisPaintopLodLimitations KisHairyBristleOptionData::lodLimitations() const
{
    KisPaintopLodLimitations l;
    l.limitations << KoID("hairy-brush", i18nc("PaintOp instant preview limitation", "Bristle Brush (the lines will be thinner than on preview)"));
    return l;
}
