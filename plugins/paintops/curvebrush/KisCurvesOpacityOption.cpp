/*
 *  SPDX-FileCopyrightText: 2022 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */


#include "KisCurvesOpacityOption.h"
#include "kis_paintop_option.h"

#include <kis_properties_configuration.h>
#include <kis_paint_information.h>
#include <kis_paintop.h>

#include <KisPaintOpOptionUtils.h>
namespace kpou = KisPaintOpOptionUtils;

KisCurvesOpacityOption::KisCurvesOpacityOption(const KisPropertiesConfiguration *setting)
    : KisCurvesOpacityOption(kpou::loadOptionData<KisCurvesOpacityOptionData>(setting))
{
}

KisCurvesOpacityOption::KisCurvesOpacityOption(const KisCurvesOpacityOptionData &data)
    : KisCurveOption2(data)
{
}


double KisCurvesOpacityOption::apply(const KisPaintInformation & info, qreal opacity) const
{
    if (!isChecked()) return opacity;
    return computeSizeLikeValue(info) * opacity;
}
