/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisPaintThicknessOption.h"

KisPaintThicknessOption::KisPaintThicknessOption(const KisPropertiesConfiguration *setting)
    : KisCurveOption2(initializeFromData(setting))
{
}

qreal KisPaintThicknessOption::apply(const KisPaintInformation & info) const
{
    if (!isChecked()) return 1.0;
    return computeSizeLikeValue(info);
}

KisPaintThicknessOptionData::ThicknessMode KisPaintThicknessOption::mode() const
{
    return m_mode;
}

KisPaintThicknessOptionData KisPaintThicknessOption::initializeFromData(const KisPropertiesConfiguration *setting)
{
    KisPaintThicknessOptionData data;
    data.read(setting);

    m_mode = data.mode;

    return data;
}
