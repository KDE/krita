/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISSCATTEROPTIONDATA_H
#define KISSCATTEROPTIONDATA_H

#include "KisCurveOptionData.h"

#include <KisOptionTuple.h>
#include <KisPrefixedOptionDataWrapper.h>


struct PAINTOP_EXPORT KisScatterOptionMixInImpl
    : boost::equality_comparable<KisScatterOptionMixInImpl>
{
    inline friend bool operator==(const KisScatterOptionMixInImpl &lhs, const KisScatterOptionMixInImpl &rhs) {
            return lhs.axisX == rhs.axisX &&
            lhs.axisY == rhs.axisY;
    }

    bool axisX {true};
    bool axisY {true};

    bool read(const KisPropertiesConfiguration *setting);
    void write(KisPropertiesConfiguration *setting) const;
};

using KisScatterOptionMixIn = KisPrefixedOptionDataWrapper<KisScatterOptionMixInImpl>;

struct PAINTOP_EXPORT KisScatterOptionData : KisOptionTuple<KisCurveOptionData, KisScatterOptionMixIn>
{
    KisScatterOptionData(const QString &prefix = "");
};


#endif // KISSCATTEROPTIONDATA_H
