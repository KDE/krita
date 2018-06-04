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

#include <KoCompositeOpRegistry.h>


KisMaskingBrushOptionProperties::KisMaskingBrushOptionProperties()
    : compositeOpId(COMPOSITE_MULT)

{
}

void KisMaskingBrushOptionProperties::write(KisPropertiesConfiguration *setting, qreal masterBrushSize) const
{
    setting->setProperty(KisPaintOpUtils::MaskingBrushEnabledTag, isEnabled);
    setting->setProperty(KisPaintOpUtils::MaskingBrushCompositeOpTag, compositeOpId);
    setting->setProperty(KisPaintOpUtils::MaskingBrushUseMasterSizeTag, useMasterSize);

    const qreal masterSizeCoeff =
        brush && masterBrushSize > 0 ? brush->userEffectiveSize() / masterBrushSize : 1.0;

    setting->setProperty(KisPaintOpUtils::MaskingBrushMasterSizeCoeffTag, masterSizeCoeff);

    // TODO: skip saving in some cases?
    // if (!isEnabled) return;

    if (brush) {
        KisPropertiesConfigurationSP embeddedConfig = new KisPropertiesConfiguration();

        {
            KisBrushOptionProperties option;
            option.setBrush(brush);
            option.writeOptionSetting(embeddedConfig);
        }

        setting->setPrefixedProperties(KisPaintOpUtils::MaskingBrushPresetPrefix, embeddedConfig);

        const QString brushFileName = brush->shortFilename();

        if (!brushFileName.isEmpty()) {
            QStringList requiredFiles =
                setting->getStringList(KisPaintOpUtils::RequiredBrushFilesListTag);
            requiredFiles << brushFileName;
            setting->setProperty(KisPaintOpUtils::RequiredBrushFilesListTag, requiredFiles);
        }
    }
}

void KisMaskingBrushOptionProperties::read(const KisPropertiesConfiguration *setting, qreal masterBrushSize)
{
    isEnabled = setting->getBool(KisPaintOpUtils::MaskingBrushEnabledTag);
    compositeOpId = setting->getString(KisPaintOpUtils::MaskingBrushCompositeOpTag, COMPOSITE_MULT);
    useMasterSize = setting->getBool(KisPaintOpUtils::MaskingBrushUseMasterSizeTag, true);

    KisPropertiesConfigurationSP embeddedConfig = new KisPropertiesConfiguration();
    setting->getPrefixedProperties(KisPaintOpUtils::MaskingBrushPresetPrefix, embeddedConfig);

    KisBrushOptionProperties option;
    option.readOptionSetting(embeddedConfig);

    brush = option.brush();

    if (brush && useMasterSize) {
        const qreal masterSizeCoeff = setting->getDouble(KisPaintOpUtils::MaskingBrushMasterSizeCoeffTag, 1.0);
        brush->setUserEffectiveSize(masterSizeCoeff * masterBrushSize);
    }
}
