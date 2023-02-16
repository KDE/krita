/*
 * SPDX-FileCopyrightText: 2022 Sharaf Zaman <shzam@sdf.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KisDuplicateOptionData.h"

#include <kis_properties_configuration.h>

bool KisDuplicateOptionData::read(const KisPropertiesConfiguration *setting)
{
    healing = setting->getBool(DUPLICATE_HEALING, false);
    correctPerspective = setting->getBool(DUPLICATE_CORRECT_PERSPECTIVE, false);
    moveSourcePoint = setting->getBool(DUPLICATE_MOVE_SOURCE_POINT, true);
    resetSourcePoint = setting->getBool(DUPLICATE_RESET_SOURCE_POINT, false);
    cloneFromProjection = setting->getBool(DUPLICATE_CLONE_FROM_PROJECTION, false);

    return true;
}

void KisDuplicateOptionData::write(KisPropertiesConfiguration *setting) const
{
    setting->setProperty(DUPLICATE_HEALING, healing);
    setting->setProperty(DUPLICATE_CORRECT_PERSPECTIVE, correctPerspective);
    setting->setProperty(DUPLICATE_MOVE_SOURCE_POINT, moveSourcePoint);
    setting->setProperty(DUPLICATE_RESET_SOURCE_POINT, resetSourcePoint);
    setting->setProperty(DUPLICATE_CLONE_FROM_PROJECTION, cloneFromProjection);
}
