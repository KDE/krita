/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisScatterOptionData.h"

#include <kis_paintop_settings.h>

const QString SCATTER_X = "Scattering/AxisX";
const QString SCATTER_Y = "Scattering/AxisY";

bool KisScatterOptionMixInImpl::read(const KisPropertiesConfiguration *setting)
{
    axisX = setting->getBool(SCATTER_X, true);
    axisY = setting->getBool(SCATTER_Y, true);

    // NOTE: the support of SCATTER_AMOUNT legacy option has
    //       been removed during the lager refactoring

    return true;
}

void KisScatterOptionMixInImpl::write(KisPropertiesConfiguration *setting) const
{
    setting->setProperty(SCATTER_X, axisX);
    setting->setProperty(SCATTER_Y, axisY);
}
