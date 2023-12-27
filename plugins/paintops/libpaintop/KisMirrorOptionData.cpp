/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisMirrorOptionData.h"

const QString MIRROR_HORIZONTAL_ENABLED = "HorizontalMirrorEnabled";
const QString MIRROR_VERTICAL_ENABLED = "VerticalMirrorEnabled";


bool KisMirrorOptionMixInImpl::read(const KisPropertiesConfiguration *setting)
{
    enableHorizontalMirror = setting->getBool(MIRROR_HORIZONTAL_ENABLED, false);
    enableVerticalMirror = setting->getBool(MIRROR_VERTICAL_ENABLED, false);
    return true;
}

void KisMirrorOptionMixInImpl::write(KisPropertiesConfiguration *setting) const
{
    setting->setProperty(MIRROR_HORIZONTAL_ENABLED, enableHorizontalMirror);
    setting->setProperty(MIRROR_VERTICAL_ENABLED, enableVerticalMirror);
}
