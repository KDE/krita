/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_pressure_texture_strength_option.h"

#include <klocale.h>

KisPressureTextureStrengthOption::KisPressureTextureStrengthOption()
    : KisCurveOption(i18n("Strength"), "Texture/Strength/", KisPaintOpOption::textureCategory(), false)
{
    setMinimumLabel(i18n("Weak"));
    setMaximumLabel(i18n("Strong"));
}

double KisPressureTextureStrengthOption::apply(const KisPaintInformation & info) const
{
    if (!isChecked()) return 1.0;
    return computeValue(info);
}

void KisPressureTextureStrengthOption::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    KisCurveOption::readOptionSetting(setting);

    /**
     * Backward compatibility with Krita < 2.7.
     *
     * Process the presets created with the old UI, when the
     * strength was a part of Texture/Pattern option.
     */
    int strengthVersion = setting->getInt("Texture/Strength/StrengthVersion", 1);
    if (strengthVersion == 1) {
        double legacyStrength = setting->getDouble("Texture/Pattern/Strength", 1.0);
        setChecked(true);
        setValue(legacyStrength);
    }
}

void KisPressureTextureStrengthOption::writeOptionSetting(KisPropertiesConfiguration* setting) const
{
    KisCurveOption::writeOptionSetting(setting);

    /**
     * Forward compatibility with the Krita < 2.7
     *
     * Duplicate the value of the maximum strength into the
     * property used by older versions of Krita.
     */
    setting->setProperty("Texture/Strength/StrengthVersion", 2);
    if (isChecked()) {
        setting->setProperty("Texture/Pattern/Strength", value());
    }
}
