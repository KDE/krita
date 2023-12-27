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

    const int value = setting->getInt("PaintOpAction", 2);
    paintingMode = value == 1 ? enumPaintingMode::BUILDUP : enumPaintingMode::WASH;

    return true;
}

void KisPaintingModeOptionData::write(KisPropertiesConfiguration *setting) const
{
    const int value =
        paintingMode == enumPaintingMode::WASH ? 2 : 1;

    setting->setProperty("PaintOpAction", value);
}
