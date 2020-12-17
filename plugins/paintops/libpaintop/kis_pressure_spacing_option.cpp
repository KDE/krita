/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2011 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kis_pressure_spacing_option.h"

#include <klocalizedstring.h>
#include "kis_paintop_settings.h"

const QString ISOTROPIC_SPACING = "Spacing/Isotropic";

KisPressureSpacingOption::KisPressureSpacingOption()
    : KisCurveOption("Spacing", KisPaintOpOption::GENERAL, false),
      m_isotropicSpacing(false),
      m_useSpacingUpdates(false)
{
}


double KisPressureSpacingOption::apply(const KisPaintInformation & info) const
{
    if (!isChecked()) return 1.0;
    return computeSizeLikeValue(info);
}

void KisPressureSpacingOption::setIsotropicSpacing(bool isotropic)
{
    m_isotropicSpacing = isotropic;
}

bool KisPressureSpacingOption::isotropicSpacing() const
{
    return m_isotropicSpacing;
}

void KisPressureSpacingOption::setUsingSpacingUpdates(bool useUpdates)
{
    m_useSpacingUpdates = useUpdates;
}

bool KisPressureSpacingOption::usingSpacingUpdates() const
{
    return m_useSpacingUpdates;
}

void KisPressureSpacingOption::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    KisCurveOption::writeOptionSetting(setting);
    setting->setProperty(ISOTROPIC_SPACING, m_isotropicSpacing);
    setting->setProperty(SPACING_USE_UPDATES, m_useSpacingUpdates);
}

void KisPressureSpacingOption::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisCurveOption::readOptionSetting(setting);
    m_isotropicSpacing = setting->getBool(ISOTROPIC_SPACING, false);
    m_useSpacingUpdates = setting->getBool(SPACING_USE_UPDATES, false);
}
