/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISSHARPNESSOPTIONDATA_H
#define KISSHARPNESSOPTIONDATA_H

#include "KisCurveOptionData.h"

#include <KisOptionTuple.h>


class PAINTOP_EXPORT KisSharpnessOptionMixIn : public boost::equality_comparable<KisSharpnessOptionMixIn>
{
public:
    inline friend bool operator==(const KisSharpnessOptionMixIn &lhs, const KisSharpnessOptionMixIn &rhs) {
            return lhs.alignOutlinePixels == rhs.alignOutlinePixels &&
            lhs.softness == rhs.softness;
    }

    bool alignOutlinePixels {false};
    int softness {0};

    bool read(const KisPropertiesConfiguration *setting);
    void write(KisPropertiesConfiguration *setting) const;
};

struct PAINTOP_EXPORT KisSharpnessOptionData : public KisOptionTuple<KisCurveOptionData, KisSharpnessOptionMixIn>
{
    KisSharpnessOptionData(const QString &prefix = "")
        : KisOptionTuple<KisCurveOptionData, KisSharpnessOptionMixIn>(KoID("Sharpness", i18n("Sharpness")))
    {
        this->prefix = prefix;
    }
};

#endif // KISSHARPNESSOPTIONDATA_H
