/*
 *  SPDX-FileCopyrightText: 2022 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_CURVES_OPACITY_OPTION_DATA_H
#define KIS_CURVES_OPACITY_OPTION_DATA_H

#include <KisCurveOptionData.h>
#include <KisCurveOption2.h>


struct KisCurvesOpacityOptionData : KisCurveOptionData
{
    KisCurvesOpacityOptionData()
        : KisCurveOptionData(
              KoID("Curves opacity", i18n("Curves opacity")),
              true, false, false,
              0.1, 1.0)
    {}
};


class KisCurvesOpacityOption : public KisCurveOption2
{
public:
    KisCurvesOpacityOption(const KisPropertiesConfiguration *setting);
    
    double apply(const KisPaintInformation & info, qreal opacity) const;
    
private:
    KisCurvesOpacityOption(const KisCurvesOpacityOptionData &data);
};

#endif // KIS_CURVES_OPACITY_OPTION_DATA_H
