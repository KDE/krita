/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISMIRROROPTIONDATA_H
#define KISMIRROROPTIONDATA_H

#include "KisCurveOptionData.h"

#include <KisOptionTuple.h>
#include <KisPrefixedOptionDataWrapper.h>

struct PAINTOP_EXPORT KisMirrorOptionMixInImpl
    : boost::equality_comparable<KisMirrorOptionMixInImpl>
{
    inline friend bool operator==(const KisMirrorOptionMixInImpl &lhs, const KisMirrorOptionMixInImpl &rhs) {
            return lhs.enableVerticalMirror == rhs.enableVerticalMirror &&
            lhs.enableHorizontalMirror == rhs.enableHorizontalMirror;
    }

    bool enableVerticalMirror {false};
    bool enableHorizontalMirror {false};

    bool read(const KisPropertiesConfiguration *setting);
    void write(KisPropertiesConfiguration *setting) const;
};

using KisMirrorOptionMixIn = KisPrefixedOptionDataWrapper<KisMirrorOptionMixInImpl>;

struct PAINTOP_EXPORT KisMirrorOptionData : KisOptionTuple<KisCurveOptionData, KisMirrorOptionMixIn>
{
    KisMirrorOptionData(const QString &prefix = "")
        : KisOptionTuple<KisCurveOptionData, KisMirrorOptionMixIn>(prefix, KoID("Mirror", i18n("Mirror")))
    {
    }
};


#endif // KISMIRROROPTIONDATA_H
