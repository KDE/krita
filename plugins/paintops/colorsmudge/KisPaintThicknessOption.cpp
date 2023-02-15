/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisPaintThicknessOption.h"

#include <KisPaintOpOptionUtils.h>
namespace kpou = KisPaintOpOptionUtils;


KisPaintThicknessOption::KisPaintThicknessOption(const KisPropertiesConfiguration *setting)
    : KisPaintThicknessOption(kpou::loadOptionData<KisPaintThicknessOptionData>(setting))
{
}

KisPaintThicknessOption::KisPaintThicknessOption(const KisPaintThicknessOptionData &data)
    : KisCurveOption(data),
      m_mode(data.mode)
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
