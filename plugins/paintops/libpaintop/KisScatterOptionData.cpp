/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisScatterOptionData.h"

#include <kis_paintop_settings.h>

const QString SCATTER_X = "Scattering/AxisX";
const QString SCATTER_Y = "Scattering/AxisY";
const QString SCATTER_AMOUNT = "Scattering/Amount";

bool KisScatterOptionMixInImpl::read(const KisPropertiesConfiguration *setting)
{
    axisX = setting->getBool(SCATTER_X, true);
    axisY = setting->getBool(SCATTER_Y, true);

    return true;
}

void KisScatterOptionMixInImpl::write(KisPropertiesConfiguration *setting) const
{
    setting->setProperty(SCATTER_X, axisX);
    setting->setProperty(SCATTER_Y, axisY);
}

KisScatterOptionData::KisScatterOptionData(const QString &prefix)
    : KisOptionTuple<KisCurveOptionData, KisScatterOptionMixIn>(prefix,
                                                                KoID("Scatter", i18n("Scatter")),
                                                                Checkability::Checkable,
                                                                std::nullopt,
                                                                std::make_pair(0.0, 5.0))
{
    valueFixUpReadCallback = [] (KisCurveOptionDataCommon *data, const KisPropertiesConfiguration *setting) {

        if (setting->hasProperty(SCATTER_AMOUNT) && !setting->hasProperty("ScatterValue")) {
            data->strengthValue = setting->getDouble(SCATTER_AMOUNT);
        }

    };
}
