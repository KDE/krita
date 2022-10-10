/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisSizeOptionData.h"


KisSizeOptionData::KisSizeOptionData(bool isCheckable, const QString &prefix)
    : KisCurveOptionData(
          KoID("Size", i18n("Size")),
          isCheckable,
          !isCheckable)
{
    this->prefix = prefix;
}

KisPaintopLodLimitations KisSizeOptionData::lodLimitations() const
{
    KisPaintopLodLimitations l;

    if (!isCheckable || isChecked) {
        // HINT: FUZZY_PER_STROKE doesn't affect instant preview
        if (sensorFuzzyPerDab.isActive) {
            l.limitations << KoID("size-fade", i18nc("PaintOp instant preview limitation", "Size -> Fuzzy (sensor)"));
        }

        if (sensorFade.isActive) {
            l.blockers << KoID("size-fuzzy", i18nc("PaintOp instant preview limitation", "Size -> Fade (sensor)"));
        }
    }

    return l;
}
