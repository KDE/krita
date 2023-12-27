/*
 *  SPDX-FileCopyrightText: 2008-2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisHairyInkOptionData.h"

#include "kis_properties_configuration.h"


const QString HAIRY_INK_DEPLETION_ENABLED = "HairyInk/enabled";
const QString HAIRY_INK_AMOUNT = "HairyInk/inkAmount";
const QString HAIRY_INK_USE_SATURATION = "HairyInk/useSaturation";
const QString HAIRY_INK_USE_OPACITY = "HairyInk/useOpacity";
const QString HAIRY_INK_USE_WEIGHTS = "HairyInk/useWeights";
const QString HAIRY_INK_PRESSURE_WEIGHT = "HairyInk/pressureWeights";
const QString HAIRY_INK_BRISTLE_LENGTH_WEIGHT = "HairyInk/bristleLengthWeights";
const QString HAIRY_INK_BRISTLE_INK_AMOUNT_WEIGHT = "HairyInk/bristleInkAmountWeight";
const QString HAIRY_INK_DEPLETION_WEIGHT = "HairyInk/inkDepletionWeight";
const QString HAIRY_INK_DEPLETION_CURVE = "HairyInk/inkDepletionCurve";
const QString HAIRY_INK_SOAK = "HairyInk/soak";


bool KisHairyInkOptionData::read(const KisPropertiesConfiguration *setting)
{
    inkDepletionEnabled = setting->getBool(HAIRY_INK_DEPLETION_ENABLED, false);
    inkAmount = setting->getInt(HAIRY_INK_AMOUNT, 1024);
    useSaturation = setting->getBool(HAIRY_INK_USE_SATURATION, false);
    useOpacity = setting->getBool(HAIRY_INK_USE_OPACITY, true);
    useWeights = setting->getBool(HAIRY_INK_USE_WEIGHTS, false);
    pressureWeight = setting->getInt(HAIRY_INK_PRESSURE_WEIGHT, 50);
    bristleLengthWeight = setting->getInt(HAIRY_INK_BRISTLE_LENGTH_WEIGHT, 50);
    bristleInkAmountWeight = setting->getInt(HAIRY_INK_BRISTLE_INK_AMOUNT_WEIGHT, 50);
    inkDepletionWeight = setting->getInt(HAIRY_INK_DEPLETION_WEIGHT, 50);
    inkDepletionCurve = setting->getCubicCurve(HAIRY_INK_DEPLETION_CURVE, KisCubicCurve()).toString();
    useSoakInk = setting->getBool(HAIRY_INK_SOAK, false);

    return true;
}

void KisHairyInkOptionData::write(KisPropertiesConfiguration *setting) const
{
    setting->setProperty(HAIRY_INK_DEPLETION_ENABLED, inkDepletionEnabled);
    setting->setProperty(HAIRY_INK_AMOUNT, inkAmount);
    setting->setProperty(HAIRY_INK_USE_SATURATION, useSaturation);
    setting->setProperty(HAIRY_INK_USE_OPACITY, useOpacity);
    setting->setProperty(HAIRY_INK_USE_WEIGHTS, useWeights);
    setting->setProperty(HAIRY_INK_PRESSURE_WEIGHT, pressureWeight);
    setting->setProperty(HAIRY_INK_BRISTLE_LENGTH_WEIGHT, bristleLengthWeight);
    setting->setProperty(HAIRY_INK_BRISTLE_INK_AMOUNT_WEIGHT, bristleInkAmountWeight);
    setting->setProperty(HAIRY_INK_DEPLETION_WEIGHT, inkDepletionWeight);
    setting->setProperty(HAIRY_INK_DEPLETION_CURVE, QVariant::fromValue(KisCubicCurve(inkDepletionCurve)));
    setting->setProperty(HAIRY_INK_SOAK, useSoakInk);
}
