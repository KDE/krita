/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisTextureOptionData.h"

#include <kis_properties_configuration.h>
#include <kis_paintop_lod_limitations.h>

bool KisTextureOptionData::read(const KisPropertiesConfiguration *setting)
{
    textureData.read(setting);

    isEnabled = setting->getBool("Texture/Pattern/Enabled", false);
    scale = setting->getDouble("Texture/Pattern/Scale", 1.0);
    brightness = setting->getDouble("Texture/Pattern/Brightness");
    contrast = setting->getDouble("Texture/Pattern/Contrast", 1.0);
    neutralPoint = setting->getDouble("Texture/Pattern/NeutralPoint", 0.5);
    offsetX = setting->getInt("Texture/Pattern/OffsetX");
    offsetY = setting->getInt("Texture/Pattern/OffsetY");
    isRandomOffsetX = setting->getBool("Texture/Pattern/isRandomOffsetX");
    isRandomOffsetY = setting->getBool("Texture/Pattern/isRandomOffsetY");
    texturingMode = static_cast<TexturingMode>(setting->getInt("Texture/Pattern/TexturingMode", MULTIPLY));
    useSoftTexturing = setting->getBool("Texture/Pattern/UseSoftTexturing");
    cutOffPolicy = setting->getInt("Texture/Pattern/CutoffPolicy");
    cutOffLeft = setting->getInt("Texture/Pattern/CutoffLeft", 0);
    cutOffRight = setting->getInt("Texture/Pattern/CutoffRight", 255);
    invert = setting->getBool("Texture/Pattern/Invert");
    autoInvertOnErase = setting->getBool("Texture/Pattern/AutoInvertOnErase");

    return true;
}

void KisTextureOptionData::write(KisPropertiesConfiguration *setting) const
{
    if (!isEnabled) return;

    textureData.write(setting);
    setting->setProperty("Texture/Pattern/Enabled", isEnabled);
    setting->setProperty("Texture/Pattern/Scale", scale);
    setting->setProperty("Texture/Pattern/Brightness", brightness);
    setting->setProperty("Texture/Pattern/Contrast", contrast);
    setting->setProperty("Texture/Pattern/NeutralPoint", neutralPoint);
    setting->setProperty("Texture/Pattern/OffsetX", offsetX);
    setting->setProperty("Texture/Pattern/OffsetY", offsetY);
    setting->setProperty("Texture/Pattern/TexturingMode", texturingMode);
    setting->setProperty("Texture/Pattern/UseSoftTexturing", useSoftTexturing);
    setting->setProperty("Texture/Pattern/CutoffLeft", cutOffLeft);
    setting->setProperty("Texture/Pattern/CutoffRight", cutOffRight);
    setting->setProperty("Texture/Pattern/CutoffPolicy", cutOffPolicy);
    setting->setProperty("Texture/Pattern/Invert", invert);
    setting->setProperty("Texture/Pattern/AutoInvertOnErase", autoInvertOnErase);
    setting->setProperty("Texture/Pattern/isRandomOffsetX", isRandomOffsetX);
    setting->setProperty("Texture/Pattern/isRandomOffsetY", isRandomOffsetY);
}

KisPaintopLodLimitations KisTextureOptionData::lodLimitations() const
{
    KisPaintopLodLimitations l;
    if (isEnabled) {
        l.limitations << KoID("texture-pattern", i18nc("PaintOp instant preview limitation", "Texture->Pattern (low quality preview)"));
    }
    return l;
}
