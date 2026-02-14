/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisPaintingModeOptionData.h"

#include "kis_properties_configuration.h"


bool KisPaintingModeOptionData::read(const KisPropertiesConfiguration *setting)
{
    hasPaintingModeProperty = setting->hasProperty("PaintOpAction");

    switch (setting->getInt("PaintOpAction", 2)) {
    case 1:
        paintingMode = enumPaintingMode::BUILDUP;
        break;
    case 3:
        paintingMode = enumPaintingMode::SOFT;
        break;
    default:
        paintingMode = enumPaintingMode::WASH;
        break;
    }

    return true;
}

void KisPaintingModeOptionData::write(KisPropertiesConfiguration *setting) const
{
    int value;
    switch (paintingMode) {
    case enumPaintingMode::BUILDUP:
        value = 1;
        break;
    case enumPaintingMode::SOFT:
        value = 3;
        break;
    default:
        value = 2;
        break;
    }

    setting->setProperty("PaintOpAction", value);
}
