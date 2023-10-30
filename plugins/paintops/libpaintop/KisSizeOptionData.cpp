/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisSizeOptionData.h"


KisSizeOptionData::KisSizeOptionData(const QString &prefix)
    : KisCurveOptionData(prefix,
          KoID("Size", i18n("Size")),
          Checkability::Checkable)
{
}

KisPaintopLodLimitations KisSizeOptionData::lodLimitations() const
{
    KisPaintopLodLimitations l;

    if (!isCheckable || isChecked) {
        // HINT: FUZZY_PER_STROKE doesn't affect instant preview
        if (sensorStruct().sensorFuzzyPerDab.isActive) {
            l.limitations << KoID("size-fade", i18nc("PaintOp instant preview limitation", "Size -> Fuzzy (sensor)"));
        }

        if (sensorStruct().sensorFade.isActive) {
            l.blockers << KoID("size-fuzzy", i18nc("PaintOp instant preview limitation", "Size -> Fade (sensor)"));
        }
    }

    return l;
}
