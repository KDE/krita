/*
 *  SPDX-FileCopyrightText: 2022 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_LINEWIDTH_OPTION_DATA_H
#define KIS_LINEWIDTH_OPTION_DATA_H

#include <KisCurveOptionData.h>
#include <KisCurveOption2.h>


struct KisLineWidthOptionData : KisCurveOptionData
{
    KisLineWidthOptionData()
        : KisCurveOptionData(
              KoID("Line width", i18n("Line width")),
              true, false, false,
              0.1, 1.0)
    {}
};


class KisLineWidthOption : public KisCurveOption2
{
public:
    KisLineWidthOption(const KisPropertiesConfiguration *setting);
    
    double apply(const KisPaintInformation & info, double lineWidth) const;
    
private:
    KisLineWidthOption(const KisLineWidthOptionData &data);
};

#endif // KIS_LINEWIDTH_OPTION_DATA_H
