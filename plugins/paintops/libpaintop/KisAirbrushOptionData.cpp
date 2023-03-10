/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisAirbrushOptionData.h"

#include <kis_paintop_settings.h>
#include <kis_properties_configuration.h>

const qreal DEFAULT_RATE = 20.0;

bool KisAirbrushOptionData::read(const KisPropertiesConfiguration *setting)
{
    isChecked = setting->getBool(AIRBRUSH_ENABLED);
    airbrushRate = setting->getDouble(AIRBRUSH_RATE, DEFAULT_RATE);
    ignoreSpacing = setting->getBool(AIRBRUSH_IGNORE_SPACING, false);

    return true;
}

void KisAirbrushOptionData::write(KisPropertiesConfiguration *setting) const
{
    setting->setProperty(AIRBRUSH_ENABLED, isChecked);
    setting->setProperty(AIRBRUSH_RATE, airbrushRate);
    setting->setProperty(AIRBRUSH_IGNORE_SPACING, ignoreSpacing);
}
