/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISSMUDGELENGTHOPTIONDATA_H
#define KISSMUDGELENGTHOPTIONDATA_H

#include "KisCurveOptionData.h"

#include <KisOptionTuple.h>
#include <KisPrefixedOptionDataWrapper.h>

struct KisSmudgeLengthOptionMixInImpl
    : boost::equality_comparable<KisSmudgeLengthOptionMixInImpl>
{
    enum Mode { SMEARING_MODE, DULLING_MODE };

    inline friend bool operator==(const KisSmudgeLengthOptionMixInImpl &lhs, const KisSmudgeLengthOptionMixInImpl &rhs) {
        return lhs.mode == rhs.mode &&
                lhs.smearAlpha == rhs.smearAlpha &&
                lhs.useNewEngine == rhs.useNewEngine;
    }

    Mode mode {SMEARING_MODE};
    bool smearAlpha {true};
    bool useNewEngine {false};

    bool read(const KisPropertiesConfiguration *setting);
    void write(KisPropertiesConfiguration *setting) const;
};

using KisSmudgeLengthOptionMixIn = KisPrefixedOptionDataWrapper<KisSmudgeLengthOptionMixInImpl>;

struct KisSmudgeLengthOptionData : KisOptionTuple<KisCurveOptionData, KisSmudgeLengthOptionMixIn>
{
    KisSmudgeLengthOptionData(const QString &prefix = "")
        : KisOptionTuple<KisCurveOptionData, KisSmudgeLengthOptionMixIn>(prefix,
                                                                         KoID("SmudgeRate", i18n("Smudge Length")))
    {
    }
};


#endif // KISSMUDGELENGTHOPTIONDATA_H
