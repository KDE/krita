/* This file is part of the KDE project
 * Copyright (c) 2011 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
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
