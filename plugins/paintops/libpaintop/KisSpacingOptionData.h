/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISSPACINGOPTIONDATA_H
#define KISSPACINGOPTIONDATA_H

#include "KisCurveOptionData.h"

#include <KisOptionTuple.h>
#include <KisPrefixedOptionDataWrapper.h>


struct PAINTOP_EXPORT KisSpacingOptionMixInImpl
    : boost::equality_comparable<KisSpacingOptionMixInImpl>
{
    inline friend bool operator==(const KisSpacingOptionMixInImpl &lhs, const KisSpacingOptionMixInImpl &rhs) {
            return lhs.useSpacingUpdates == rhs.useSpacingUpdates &&
            lhs.isotropicSpacing == rhs.isotropicSpacing;
    }

    bool useSpacingUpdates {false};
    bool isotropicSpacing {false};

    bool read(const KisPropertiesConfiguration *setting);
    void write(KisPropertiesConfiguration *setting) const;
};

using KisSpacingOptionMixIn = KisPrefixedOptionDataWrapper<KisSpacingOptionMixInImpl>;

struct PAINTOP_EXPORT KisSpacingOptionData : KisOptionTuple<KisCurveOptionData, KisSpacingOptionMixIn>
{
    KisSpacingOptionData(const QString &prefix = "")
        : KisOptionTuple<KisCurveOptionData, KisSpacingOptionMixIn>(prefix, KoID("Spacing", i18n("Spacing")))
    {
    }
};


#endif // KISSPACINGOPTIONDATA_H
