/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisGradientOption.h"

#include <KisColorSmudgeStandardOptionData.h>
#include <KoColor.h>
#include <resources/KoAbstractGradient.h>

#include <KisPaintOpOptionUtils.h>
namespace kpou = KisPaintOpOptionUtils;


KisGradientOption::KisGradientOption(const KisPropertiesConfiguration *setting)
    : KisCurveOption(kpou::loadOptionData<KisGradientOptionData>(setting))
{
}

void KisGradientOption::apply(KoColor& color, const KoAbstractGradientSP gradient, const KisPaintInformation& info) const
{
    if (isChecked() && gradient) {
        gradient->colorAt(color, computeSizeLikeValue(info));
    }
}
