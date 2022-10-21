/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISPAINTTHICKNESSOPTIONDATA_H
#define KISPAINTTHICKNESSOPTIONDATA_H

#include "KisCurveOptionData.h"

#include <KisOptionTuple.h>
#include <KisPrefixedOptionDataWrapper.h>


struct PAINTOP_EXPORT KisPaintThicknessOptionMixInImpl
    : boost::equality_comparable<KisPaintThicknessOptionMixInImpl>
{
    enum ThicknessMode {
        RESERVED,
        OVERWRITE,
        OVERLAY
    };

    inline friend bool operator==(const KisPaintThicknessOptionMixInImpl &lhs, const KisPaintThicknessOptionMixInImpl &rhs) {
            return lhs.mode == rhs.mode;
    }

    ThicknessMode mode;

    bool read(const KisPropertiesConfiguration *setting);
    void write(KisPropertiesConfiguration *setting) const;
};

using KisPaintThicknessOptionMixIn = KisPrefixedOptionDataWrapper<KisPaintThicknessOptionMixInImpl>;

struct PAINTOP_EXPORT KisPaintThicknessOptionData : KisOptionTuple<KisCurveOptionData, KisPaintThicknessOptionMixIn>
{
    KisPaintThicknessOptionData(const QString &prefix = "")
        : KisOptionTuple<KisCurveOptionData, KisPaintThicknessOptionMixIn>(prefix,
                                                                           KoID("PaintThickness", i18n("Paint Thickness")))
    {
    }
};

#endif // KISPAINTTHICKNESSOPTIONDATA_H
