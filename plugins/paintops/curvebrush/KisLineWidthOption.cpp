/*
 *  SPDX-FileCopyrightText: 2022 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */


#include "KisLineWidthOption.h"
#include "kis_paintop_option.h"

#include <kis_properties_configuration.h>
#include <kis_paint_information.h>
#include <kis_paintop.h>

#include <KisPaintOpOptionUtils.h>
namespace kpou = KisPaintOpOptionUtils;

KisLineWidthOption::KisLineWidthOption(const KisPropertiesConfiguration *setting)
    : KisLineWidthOption(kpou::loadOptionData<KisLineWidthOptionData>(setting))
{
}

KisLineWidthOption::KisLineWidthOption(const KisLineWidthOptionData &data)
    : KisCurveOption2(data)
{
}


double KisLineWidthOption::apply(const KisPaintInformation & info, double lineWidth) const
{
    if (!isChecked()) return lineWidth;
    return computeSizeLikeValue(info) * lineWidth;
}
