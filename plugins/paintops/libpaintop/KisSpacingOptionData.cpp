/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisSpacingOptionData.h"

#include <kis_paintop_settings.h>

const QString ISOTROPIC_SPACING = "Spacing/Isotropic";

bool KisSpacingOptionMixInImpl::read(const KisPropertiesConfiguration *setting)
{
    isotropicSpacing = setting->getBool(ISOTROPIC_SPACING, false);
    useSpacingUpdates = setting->getBool(SPACING_USE_UPDATES, false);

    return true;
}

void KisSpacingOptionMixInImpl::write(KisPropertiesConfiguration *setting) const
{
    setting->setProperty(ISOTROPIC_SPACING, isotropicSpacing);
    setting->setProperty(SPACING_USE_UPDATES, useSpacingUpdates);
}
