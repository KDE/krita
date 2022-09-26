/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISSCATTEROPTIONDATA_H
#define KISSCATTEROPTIONDATA_H

#include "KisCurveOptionData.h"

#include <KisOptionTuple.h>

class PAINTOP_EXPORT KisScatterOptionMixIn : boost::equality_comparable<KisScatterOptionMixIn>
{
public:
    inline friend bool operator==(const KisScatterOptionMixIn &lhs, const KisScatterOptionMixIn &rhs) {
            return lhs.axisX == rhs.axisX &&
            lhs.axisY == rhs.axisY;
    }

    bool axisX {true};
    bool axisY {true};

    bool read(const KisPropertiesConfiguration *setting);
    void write(KisPropertiesConfiguration *setting) const;
};

struct PAINTOP_EXPORT KisScatterOptionData : public KisOptionTuple<KisCurveOptionData, KisScatterOptionMixIn>
{
    KisScatterOptionData(const QString &prefix = "")
        : KisOptionTuple<KisCurveOptionData, KisScatterOptionMixIn>(KoID("Scatter", i18n("Scatter")),
                                                                    true, false, false, 0.0, 5.0)
    {
        this->prefix = prefix;
    }
};


#endif // KISSCATTEROPTIONDATA_H
