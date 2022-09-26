/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISMIRROROPTIONDATA_H
#define KISMIRROROPTIONDATA_H

#include "KisCurveOptionData.h"

#include <KisOptionTuple.h>


class PAINTOP_EXPORT KisMirrorOptionMixIn : public boost::equality_comparable<KisMirrorOptionMixIn>
{
public:
    inline friend bool operator==(const KisMirrorOptionMixIn &lhs, const KisMirrorOptionMixIn &rhs) {
            return lhs.enableVerticalMirror == rhs.enableVerticalMirror &&
            lhs.enableHorizontalMirror == rhs.enableHorizontalMirror;
    }

    bool enableVerticalMirror {false};
    bool enableHorizontalMirror {false};

    bool read(const KisPropertiesConfiguration *setting);
    void write(KisPropertiesConfiguration *setting) const;
};

struct PAINTOP_EXPORT KisMirrorOptionData : public KisOptionTuple<KisCurveOptionData, KisMirrorOptionMixIn>
{
    KisMirrorOptionData(const QString &prefix = "")
        : KisOptionTuple<KisCurveOptionData, KisMirrorOptionMixIn>(KoID("Mirror", i18n("Mirror")))
    {
        this->prefix = prefix;
    }
};


#endif // KISMIRROROPTIONDATA_H
