/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISSTANDARDOPTIONDATA_H
#define KISSTANDARDOPTIONDATA_H

#include <KisCurveOptionData.h>


class KisOpacityOptionData : public KisCurveOptionData
{
public:
    KisOpacityOptionData(bool isCheckable = false, const QString &prefix = QString())
        : KisCurveOptionData(
              KoID("Opacity", i18n("Opacity")),
              isCheckable, !isCheckable)
    {
        this->prefix = prefix;
    }
};

class KisFlowOptionData : public KisCurveOptionData
{
public:
    KisFlowOptionData(bool isCheckable = false, const QString &prefix = QString())
        : KisCurveOptionData(
              KoID("Flow", i18n("Flow")),
              isCheckable,
              !isCheckable)
    {
        this->prefix = prefix;
    }
};


class KisRatioOptionData : public KisCurveOptionData
{
public:
    KisRatioOptionData(const QString &prefix = QString())
        : KisCurveOptionData(
              KoID("Ratio", i18n("Ratio")))
    {
        this->prefix = prefix;
    }
};

class KisSoftnessOptionData : public KisCurveOptionData
{
public:
    KisSoftnessOptionData()
        : KisCurveOptionData(
              KoID("Softness", i18n("Softness")),
              true, false, false,
              0.1, 1.0)
    {}
};

class KisRotationOptionData : public KisCurveOptionData
{
public:
    KisRotationOptionData(const QString &prefix = QString())
        : KisCurveOptionData(
              KoID("Rotation", i18n("Rotation")))
    {
        this->prefix = prefix;
    }
};

class KisDarkenOptionData : public KisCurveOptionData
{
public:
    KisDarkenOptionData()
        : KisCurveOptionData(
              KoID("Darken", i18n("Darken")))
    {}
};

class KisMixOptionData : public KisCurveOptionData
{
public:
    KisMixOptionData()
        : KisCurveOptionData(
              KoID("Mix", i18nc("Mixing of colors", "Mix")))
    {}
};

class KisHueOptionData : public KisCurveOptionData
{
public:
    KisHueOptionData()
        : KisCurveOptionData(
              KoID("h", i18n("Hue")))
    {}
};

class KisSaturationOptionData : public KisCurveOptionData
{
public:
    KisSaturationOptionData()
        : KisCurveOptionData(
              KoID("s", i18n("Saturation")))
    {}
};

class KisValueOptionData : public KisCurveOptionData
{
public:
    KisValueOptionData()
        : KisCurveOptionData(
              KoID("v", i18nc("Label of Brightness value in Color Smudge brush engine options", "Value")))
    {}
};

class KisRateOptionData : public KisCurveOptionData
{
public:
    KisRateOptionData()
        : KisCurveOptionData(
              KoID("Rate", i18n("Rate")))
    {}
};

class KisStrengthOptionData : public KisCurveOptionData
{
public:
    KisStrengthOptionData()
        : KisCurveOptionData(
              KoID("Texture/Strength/", i18n("Strength")))
    {}
};

class KisCurveOptionWidget2;

namespace KisPaintOpOptionUtils {

PAINTOP_EXPORT KisCurveOptionWidget2* createOpacityOptionWidget();
PAINTOP_EXPORT KisCurveOptionWidget2* createFlowOptionWidget();
PAINTOP_EXPORT KisCurveOptionWidget2* createRatioOptionWidget();
PAINTOP_EXPORT KisCurveOptionWidget2* createSoftnessOptionWidget();
PAINTOP_EXPORT KisCurveOptionWidget2* createRotationOptionWidget();
PAINTOP_EXPORT KisCurveOptionWidget2* createDarkenOptionWidget();
PAINTOP_EXPORT KisCurveOptionWidget2* createMixOptionWidget();
PAINTOP_EXPORT KisCurveOptionWidget2* createHueOptionWidget();
PAINTOP_EXPORT KisCurveOptionWidget2* createSaturationOptionWidget();
PAINTOP_EXPORT KisCurveOptionWidget2* createValueOptionWidget();
PAINTOP_EXPORT KisCurveOptionWidget2* createRateOptionWidget();
PAINTOP_EXPORT KisCurveOptionWidget2* createStrengthOptionWidget();

PAINTOP_EXPORT KisCurveOptionWidget2* createMaskingOpacityOptionWidget();
PAINTOP_EXPORT KisCurveOptionWidget2* createMaskingSizeOptionWidget();
PAINTOP_EXPORT KisCurveOptionWidget2* createMaskingFlowOptionWidget();
PAINTOP_EXPORT KisCurveOptionWidget2* createMaskingRatioOptionWidget();
PAINTOP_EXPORT KisCurveOptionWidget2* createMaskingRotationOptionWidget();
PAINTOP_EXPORT KisCurveOptionWidget2* createMaskingMirrorOptionWidget();
PAINTOP_EXPORT KisCurveOptionWidget2* createMaskingScatterOptionWidget();

}

#endif // KISSTANDARDOPTIONDATA_H
