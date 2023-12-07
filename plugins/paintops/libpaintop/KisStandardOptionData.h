/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISSTANDARDOPTIONDATA_H
#define KISSTANDARDOPTIONDATA_H

#include <KisCurveOptionData.h>


struct KisOpacityOptionData : KisCurveOptionData
{
    KisOpacityOptionData(const QString &prefix = QString())
        : KisCurveOptionData(
              prefix,
              KoID("Opacity", i18n("Opacity")),
              KisKritaSensorPack::Checkability::NotCheckable)
    {
    }
};

struct KisFlowOptionData : KisCurveOptionData
{
    KisFlowOptionData(const QString &prefix = QString())
        : KisCurveOptionData(
              prefix,
              KoID("Flow", i18n("Flow")),
              KisKritaSensorPack::Checkability::NotCheckable)
    {
    }
};


struct KisRatioOptionData : KisCurveOptionData
{
    KisRatioOptionData(const QString &prefix = QString())
        : KisCurveOptionData(
              prefix,
              KoID("Ratio", i18n("Ratio")))
    {
    }
};

struct KisSoftnessOptionData : KisCurveOptionData
{
    KisSoftnessOptionData()
        : KisCurveOptionData(
              KoID("Softness", i18n("Softness")),
              Checkability::Checkable, std::nullopt,
              std::make_pair(0.1, 1.0))
    {}
};

struct KisRotationOptionData : KisCurveOptionData
{
    KisRotationOptionData(const QString &prefix = QString())
        : KisCurveOptionData(
              prefix,
              KoID("Rotation", i18n("Rotation")))
    {
    }
};

struct KisDarkenOptionData : KisCurveOptionData
{
    KisDarkenOptionData()
        : KisCurveOptionData(
              KoID("Darken", i18n("Darken")))
    {}
};

struct KisMixOptionData : KisCurveOptionData
{
    KisMixOptionData()
        : KisCurveOptionData(
              KoID("Mix", i18nc("Mixing of colors", "Mix")))
    {}
};

struct KisHueOptionData : KisCurveOptionData
{
    KisHueOptionData()
        : KisCurveOptionData(
              KoID("h", i18n("Hue")))
    {}
};

struct KisSaturationOptionData : KisCurveOptionData
{
    KisSaturationOptionData()
        : KisCurveOptionData(
              KoID("s", i18n("Saturation")))
    {}
};

struct KisValueOptionData : KisCurveOptionData
{
    KisValueOptionData()
        : KisCurveOptionData(
              KoID("v", i18nc("Label of Brightness value in Color Smudge brush engine options", "Value")))
    {}
};

struct KisRateOptionData : KisCurveOptionData
{
    KisRateOptionData()
        : KisCurveOptionData(
              KoID("Rate", i18n("Rate")))
    {}
};

struct KisStrengthOptionData : KisCurveOptionData
{
    KisStrengthOptionData()
        : KisCurveOptionData(
              KoID("Texture/Strength/", i18n("Strength")))
    {}
};

struct KisLightnessStrengthOptionData : KisCurveOptionData
{
    KisLightnessStrengthOptionData()
        : KisCurveOptionData(
              KoID("LightnessStrength", i18n("Lightness Strength")))
    {
    }
};


class KisCurveOptionWidget;

namespace KisPaintOpOptionWidgetUtils {

PAINTOP_EXPORT KisCurveOptionWidget* createOpacityOptionWidget();
PAINTOP_EXPORT KisCurveOptionWidget* createFlowOptionWidget();
PAINTOP_EXPORT KisCurveOptionWidget* createRatioOptionWidget();
PAINTOP_EXPORT KisCurveOptionWidget* createSoftnessOptionWidget();
PAINTOP_EXPORT KisCurveOptionWidget* createRotationOptionWidget();
PAINTOP_EXPORT KisCurveOptionWidget* createDarkenOptionWidget();
PAINTOP_EXPORT KisCurveOptionWidget* createMixOptionWidget();
PAINTOP_EXPORT KisCurveOptionWidget* createHueOptionWidget();
PAINTOP_EXPORT KisCurveOptionWidget* createSaturationOptionWidget();
PAINTOP_EXPORT KisCurveOptionWidget* createValueOptionWidget();
PAINTOP_EXPORT KisCurveOptionWidget* createRateOptionWidget();
PAINTOP_EXPORT KisCurveOptionWidget* createStrengthOptionWidget();

PAINTOP_EXPORT KisCurveOptionWidget* createMaskingOpacityOptionWidget();
PAINTOP_EXPORT KisCurveOptionWidget* createMaskingSizeOptionWidget();
PAINTOP_EXPORT KisCurveOptionWidget* createMaskingFlowOptionWidget();
PAINTOP_EXPORT KisCurveOptionWidget* createMaskingRatioOptionWidget();
PAINTOP_EXPORT KisCurveOptionWidget* createMaskingRotationOptionWidget();
PAINTOP_EXPORT KisCurveOptionWidget* createMaskingMirrorOptionWidget();
PAINTOP_EXPORT KisCurveOptionWidget* createMaskingScatterOptionWidget();

}

#endif // KISSTANDARDOPTIONDATA_H
