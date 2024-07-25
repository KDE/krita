/*
 *  SPDX-FileCopyrightText: 2022 Agata Cacko <cacko.azh@gmail.com>
 *  SPDX-FileCopyrightText: 2009, 2010 Lukáš Tvrdý (lukast.dev@gmail.com)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisColorOptionData.h"

#include "kis_properties_configuration.h"


bool KisColorOptionData::read(const KisPropertiesConfiguration *setting)
{
	hue = setting->getInt(COLOROP_HUE, 0);
    saturation = setting->getInt(COLOROP_SATURATION, 0);
    value = setting->getInt(COLOROP_VALUE, 0);
    useRandomOpacity = setting->getBool(COLOROP_USE_RANDOM_OPACITY, false);
    useRandomHSV = setting->getBool(COLOROP_USE_RANDOM_HSV, false);
    sampleInputColor = setting->getBool(COLOROP_SAMPLE_COLOR, false);
    fillBackground = setting->getBool(COLOROP_FILL_BG, false);
    colorPerParticle = setting->getBool(COLOROP_COLOR_PER_PARTICLE, false);
    mixBgColor = setting->getBool(COLOROP_MIX_BG_COLOR, false);
    return true;
}

void KisColorOptionData::write(KisPropertiesConfiguration *setting) const
{
	setting->setProperty(COLOROP_HUE, hue);
    setting->setProperty(COLOROP_SATURATION, saturation);
    setting->setProperty(COLOROP_VALUE, value);

    setting->setProperty(COLOROP_USE_RANDOM_HSV, useRandomHSV);
    setting->setProperty(COLOROP_USE_RANDOM_OPACITY, useRandomOpacity);
    setting->setProperty(COLOROP_SAMPLE_COLOR, sampleInputColor);

    setting->setProperty(COLOROP_FILL_BG, fillBackground);
    setting->setProperty(COLOROP_COLOR_PER_PARTICLE, colorPerParticle);
    setting->setProperty(COLOROP_MIX_BG_COLOR, mixBgColor);
}
