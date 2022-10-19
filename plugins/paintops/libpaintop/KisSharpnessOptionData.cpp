/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisSharpnessOptionData.h"

const QString SHARPNESS_FACTOR = "Sharpness/factor";
const QString SHARPNESS_ALIGN_OUTLINE_PIXELS = "Sharpness/alignoutline";
const QString SHARPNESS_SOFTNESS  = "Sharpness/softness";


bool KisSharpnessOptionMixInImpl::read(const KisPropertiesConfiguration *setting)
{
    alignOutlinePixels = setting->getBool(SHARPNESS_ALIGN_OUTLINE_PIXELS);
    softness = setting->getInt(SHARPNESS_SOFTNESS);

    // NOTE: the support of SHARPNESS_FACTOR legacy option has
    //       been removed during the lager refactoring

    return true;
}

void KisSharpnessOptionMixInImpl::write(KisPropertiesConfiguration *setting) const
{
    setting->setProperty(SHARPNESS_ALIGN_OUTLINE_PIXELS, alignOutlinePixels);
    setting->setProperty(SHARPNESS_SOFTNESS, softness);
}
