/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisCompositeOpOptionData.h"
#include <KoCompositeOpRegistry.h>
#include <kis_properties_configuration.h>

KisCompositeOpOptionData::KisCompositeOpOptionData()
    : compositeOpId(KoCompositeOpRegistry::instance().getDefaultCompositeOp().id())
{
}

bool KisCompositeOpOptionData::read(const KisPropertiesConfiguration *setting)
{
    compositeOpId = setting->getString("CompositeOp", KoCompositeOpRegistry::instance().getDefaultCompositeOp().id());
    eraserMode = setting->getBool("EraserMode", false);

    return true;
}

void KisCompositeOpOptionData::write(KisPropertiesConfiguration *setting) const
{
    setting->setProperty("CompositeOp", compositeOpId);
    setting->setProperty("EraserMode", eraserMode);
}
