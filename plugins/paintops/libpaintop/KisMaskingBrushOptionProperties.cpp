/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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

#include "KisMaskingBrushOptionProperties.h"

#include "kis_brush_option.h"
#include <brushengine/KisPaintopSettingsIds.h>

void KisMaskingBrushOptionProperties::write(KisPropertiesConfiguration *setting) const
{
    setting->setProperty(KisPaintOpUtils::MaskingBrushEnabledTag, isEnabled);

    // TODO: skip saving in some cases?
    // if (!isEnabled) return;

    if (brush) {
        KisPropertiesConfigurationSP embeddedConfig = new KisPropertiesConfiguration();

        {
            KisBrushOption option;
            option.setBrush(brush);
            option.writeOptionSetting(embeddedConfig);
        }

        // the masking brush should paint without any opacity or flow
        embeddedConfig->setProperty("PressureOpacity", false);
        embeddedConfig->setProperty("OpacityUseCurve", false);
        embeddedConfig->setProperty("PressureFlow", false);
        embeddedConfig->setProperty("FlowUseCurve", false);

        setting->setPrefixedProperties(KisPaintOpUtils::MaskingBrushPresetPrefix, embeddedConfig);

        // FIXME: the property should be able to contain multiple files
        // setting->setProperty("requiredBrushFile", brushFileName);
    }
}

void KisMaskingBrushOptionProperties::read(const KisPropertiesConfiguration *setting)
{
    isEnabled = setting->getBool(KisPaintOpUtils::MaskingBrushEnabledTag);

    KisPropertiesConfigurationSP embeddedConfig = new KisPropertiesConfiguration();
    setting->getPrefixedProperties(KisPaintOpUtils::MaskingBrushPresetPrefix, embeddedConfig);

    KisBrushOption option;
    option.readOptionSetting(embeddedConfig);

    brush = option.brush();
}
