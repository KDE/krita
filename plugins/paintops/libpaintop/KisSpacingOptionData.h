/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISSPACINGOPTIONDATA_H
#define KISSPACINGOPTIONDATA_H

#include "KisCurveOptionData.h"

#include <KisOptionTuple.h>


class PAINTOP_EXPORT KisSpacingOptionMixIn : boost::equality_comparable<KisSpacingOptionMixIn>
{
public:
    KisSpacingOptionMixIn();

    inline friend bool operator==(const KisSpacingOptionMixIn &lhs, const KisSpacingOptionMixIn &rhs) {
            return lhs.useSpacingUpdates == rhs.useSpacingUpdates &&
            lhs.isotropicSpacing == rhs.isotropicSpacing;
    }

    bool useSpacingUpdates {false};
    bool isotropicSpacing {false};

    bool read(const KisPropertiesConfiguration *setting);
    void write(KisPropertiesConfiguration *setting) const;
};

struct PAINTOP_EXPORT KisSpacingOptionData : public KisOptionTuple<KisCurveOptionData, KisSpacingOptionMixIn>
{
    KisSpacingOptionData()
        : KisOptionTuple<KisCurveOptionData, KisSpacingOptionMixIn>(KoID("Spacing", i18n("Spacing")))
    {
    }
};


#endif // KISSPACINGOPTIONDATA_H
