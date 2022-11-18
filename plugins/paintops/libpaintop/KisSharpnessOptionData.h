/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISSHARPNESSOPTIONDATA_H
#define KISSHARPNESSOPTIONDATA_H

#include "KisCurveOptionData.h"

#include <KisOptionTuple.h>
#include <KisPrefixedOptionDataWrapper.h>


struct PAINTOP_EXPORT KisSharpnessOptionMixInImpl
    : boost::equality_comparable<KisSharpnessOptionMixInImpl>
{
    inline friend bool operator==(const KisSharpnessOptionMixInImpl &lhs, const KisSharpnessOptionMixInImpl &rhs) {
            return lhs.alignOutlinePixels == rhs.alignOutlinePixels &&
            lhs.softness == rhs.softness;
    }

    bool alignOutlinePixels {false};
    int softness {0};

    bool read(const KisPropertiesConfiguration *setting);
    void write(KisPropertiesConfiguration *setting) const;
};

using KisSharpnessOptionMixIn = KisPrefixedOptionDataWrapper<KisSharpnessOptionMixInImpl>;

struct PAINTOP_EXPORT KisSharpnessOptionData : KisOptionTuple<KisCurveOptionData, KisSharpnessOptionMixIn>
{
    KisSharpnessOptionData(const QString &prefix = "");
};

#endif // KISSHARPNESSOPTIONDATA_H
