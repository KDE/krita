/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisLodAvailabilityData.h"

#include <kis_properties_configuration.h>
#include <kis_paintop_registry.h>

bool KisLodAvailabilityData::read(const KisPropertiesConfiguration *setting)
{
    const QString paintopId = setting->getString("paintop");
    if (paintopId.isEmpty()) {
        return false;
    }

    KisPaintOpFactory *factory = KisPaintOpRegistry::instance()->get(paintopId);
    if (!factory) {
        return false;
    }

    isLodSizeThresholdSupported = factory->lodSizeThresholdSupported();
    isLodUserAllowed = setting->getBool("lodUserAllowed", true);
    lodSizeThreshold = setting->getDouble("lodSizeThreshold", 100.0);

    return true;
}

void KisLodAvailabilityData::write(KisPropertiesConfiguration *setting) const
{
    setting->setProperty("lodUserAllowed", isLodUserAllowed);
    setting->setProperty("lodSizeThreshold", lodSizeThreshold);
}
