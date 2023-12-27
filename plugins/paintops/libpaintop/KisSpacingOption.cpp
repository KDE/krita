/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisSpacingOption.h"

#include <kis_properties_configuration.h>
#include <KisSpacingOptionData.h>

#include <KisPaintOpOptionUtils.h>
namespace kpou = KisPaintOpOptionUtils;


KisSpacingOption::KisSpacingOption(const KisPropertiesConfiguration *setting)
    : KisSpacingOption(kpou::loadOptionData<KisSpacingOptionData>(setting))
{
}

KisSpacingOption::KisSpacingOption(const KisSpacingOptionData &data)
    : KisCurveOption(data)
    , m_isotropicSpacing(data.isotropicSpacing)
    , m_useSpacingUpdates(data.useSpacingUpdates)
{
}

qreal KisSpacingOption::apply(const KisPaintInformation &info) const
{
    if (!isChecked()) return 1.0;
    return computeSizeLikeValue(info);
}

bool KisSpacingOption::isotropicSpacing() const
{
    return m_isotropicSpacing;
}

bool KisSpacingOption::usingSpacingUpdates() const
{
    return m_useSpacingUpdates;
}
