/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisMaskingBrushOptionProperties.h"

#include <brushengine/KisPaintopSettingsIds.h>

#include <kis_image_config.h>

bool KisBrushModel::operator==(const KisBrushModel::MaskingBrushData &lhs, const KisBrushModel::MaskingBrushData &rhs)
{
    return lhs.isEnabled == rhs.isEnabled &&
            lhs.brush == rhs.brush &&
            lhs.compositeOpId == rhs.compositeOpId &&
            lhs.useMasterSize == rhs.useMasterSize &&
            qFuzzyCompare(lhs.masterSizeCoeff, rhs.masterSizeCoeff);
}

KisBrushModel::MaskingBrushData KisBrushModel::MaskingBrushData::read(const KisPropertiesConfiguration *config, qreal masterBrushSize, KisResourcesInterfaceSP resourcesInterface)
{
    KisBrushModel::MaskingBrushData data;

    data.isEnabled = config->getBool(KisPaintOpUtils::MaskingBrushEnabledTag);
    data.compositeOpId = config->getString(KisPaintOpUtils::MaskingBrushCompositeOpTag, COMPOSITE_MULT);
    data.useMasterSize = config->getBool(KisPaintOpUtils::MaskingBrushUseMasterSizeTag, true);

    KisPropertiesConfigurationSP embeddedConfig = new KisPropertiesConfiguration();
    config->getPrefixedProperties(KisPaintOpUtils::MaskingBrushPresetPrefix, embeddedConfig);

    std::optional<BrushData> embeddedBrush = BrushData::read(embeddedConfig.constData(), resourcesInterface);

    if (embeddedBrush) {
        data.brush = *embeddedBrush;
    }

    if (data.useMasterSize) {
        data.masterSizeCoeff = config->getDouble(KisPaintOpUtils::MaskingBrushMasterSizeCoeffTag, 1.0);
        qreal size = data.masterSizeCoeff * masterBrushSize;

        const qreal maxMaskingBrushSize = KisImageConfig(true).maxMaskingBrushSize();

        if (size > maxMaskingBrushSize) {
            size = maxMaskingBrushSize;
        }

        KisBrushModel::setEffectiveSizeForBrush(data.brush.type,
                                                data.brush.autoBrush,
                                                data.brush.predefinedBrush,
                                                data.brush.textBrush,
                                                size);
    }

    return data;
}

void KisBrushModel::MaskingBrushData::write(KisPropertiesConfiguration *config) const
{
    config->setProperty(KisPaintOpUtils::MaskingBrushEnabledTag, isEnabled);
    config->setProperty(KisPaintOpUtils::MaskingBrushCompositeOpTag, compositeOpId);
    config->setProperty(KisPaintOpUtils::MaskingBrushUseMasterSizeTag, useMasterSize);
    config->setProperty(KisPaintOpUtils::MaskingBrushMasterSizeCoeffTag, masterSizeCoeff);

    // TODO: skip saving in some cases?
    // if (!isEnabled) return;

    {
        KisPropertiesConfigurationSP embeddedConfig = new KisPropertiesConfiguration();
        brush.write(embeddedConfig.data());
        config->setPrefixedProperties(KisPaintOpUtils::MaskingBrushPresetPrefix, embeddedConfig);
    }
}
