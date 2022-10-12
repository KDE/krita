/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISSTANDARDOPTIONS_H
#define KISSTANDARDOPTIONS_H

#include <KisCurveOption2.h>
#include "kis_properties_configuration.h"

#include <KisStandardOptionData.h>
#include <KisSizeOptionData.h>

template <typename Data>
class KisStandardOption : public KisCurveOption2
{
public:
    KisStandardOption(const KisPropertiesConfiguration *setting)
        : KisCurveOption2(initializeData(setting))
    {
    }

    qreal apply(const KisPaintInformation & info) const
    {
        if (!isChecked()) return 1.0;
        return computeSizeLikeValue(info);
    }

private:
    Data initializeData(const KisPropertiesConfiguration *setting) {
        Data data;
        data.read(setting);
        return data;
    }
};

using KisFlowOption = KisStandardOption<KisFlowOptionData>;
using KisSizeOption = KisStandardOption<KisSizeOptionData>;
using KisRatioOption = KisStandardOption<KisRatioOptionData>;
using KisRateOption = KisStandardOption<KisRateOptionData>;
using KisSoftnessOption = KisStandardOption<KisSoftnessOptionData>;
using KisLightnessStrengthOption = KisStandardOption<KisLightnessStrengthOptionData>;
using KisStrengthOption = KisStandardOption<KisStrengthOptionData>;
using KisMixOption = KisStandardOption<KisMixOptionData>;


#endif // KISSTANDARDOPTIONS_H
