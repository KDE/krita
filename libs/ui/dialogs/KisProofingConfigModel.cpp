/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisProofingConfigModel.h"
#include <KisLager.h>
#include <KisZug.h>
#include <KisPortingUtils.h>

#include <kis_display_color_converter.h>
#include <lager/lenses/tuple.hpp>
#include <lager/constant.hpp>

namespace {
auto conversionFlag = [](KoColorConversionTransformation::ConversionFlag flag) {
    return lager::lenses::getset(
        [flag] (const KoColorConversionTransformation::ConversionFlags &value) -> bool {
            return value.testFlag(flag);
        },
        [flag] (KoColorConversionTransformation::ConversionFlags value, const bool &val){
            value.setFlag(flag, val);
            return value;
        }
    );
};

KoColorConversionTransformation::Intent calcEffectiveDisplayIntent(KisProofingConfiguration::DisplayTransformState displayMode,
                               KoColorConversionTransformation::Intent localDisplayIntent,
                               KoColorConversionTransformation::Intent globalDisplayIntent)
{
    return displayMode == KisProofingConfiguration::Monitor ? globalDisplayIntent:
                                                              displayMode == KisProofingConfiguration::Paper ?
                                                                  KoColorConversionTransformation::IntentAbsoluteColorimetric: localDisplayIntent;
}

CheckBoxState calcEffectiveUseBPCState(KisProofingConfiguration::DisplayTransformState displayMode,
                                       KoColorConversionTransformation::Intent effectiveDisplayIntent,
                                       bool localUseBPC,
                                       bool globalUseBPC)
{
    if (effectiveDisplayIntent != KoColorConversionTransformation::IntentRelativeColorimetric) {
        return {false, false};
    }

    switch (displayMode) {
    case KisProofingConfiguration::Monitor:
        return {globalUseBPC, false};
    case KisProofingConfiguration::Paper:
        return {false, false};
    case KisProofingConfiguration::Custom:
        return {localUseBPC, true};
    }

    Q_UNREACHABLE_RETURN({false, false});
}

CheckBoxState calcEffectiveAdaptationSwitchState(KisProofingConfiguration::DisplayTransformState displayMode,
                                   KoColorConversionTransformation::Intent intent,
                                   bool adaptationFlag)
{
    if (displayMode != KisProofingConfiguration::Custom) return {true, false};
    if (intent != KoColorConversionTransformation::IntentAbsoluteColorimetric) return {true, false};
    return {adaptationFlag, true};
}

ComboBoxState calcIntentComboBoxState(KoColorConversionTransformation::Intent intent, bool enabled)
{
    QStringList values;
    QStringList toolTips;

    values << i18nc("Color conversion intent", "Perceptual");
    values << i18nc("Color conversion intent", "Relative Colorimetric");
    values << i18nc("Color conversion intent", "Saturation");
    values << i18nc("Color conversion intent", "Absolute Colorimetric");

    // no tooltops for now
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    toolTips << "" << "" << "" << "";
#else
    toolTips.resize(4);
#endif

    return {values, static_cast<int>(intent), enabled, toolTips};
}

ComboBoxState calcModeComboBoxState(KisProofingConfiguration::DisplayTransformState mode)
{
    QStringList values;
    QStringList toolTips;

    values << i18nc("Display Mode", "Use global display settings");
    values << i18nc("Display Mode", "Simulate paper white and black");
    values << i18nc("Display Mode", "Custom");

    toolTips << i18nc("@info:tooltip", "Use Rendering Intent, Blackpoint compensation and Adaptation set in the color management configuration.");
    toolTips << i18nc("@info:tooltip", "Simulate paper by using absolute colorimetric and disabling white point adaptation and blackpoint compensation.");
    toolTips << i18nc("@info:tooltip", "Select custom settings for the second transform.");

    return {values, static_cast<int>(mode), true, toolTips};
}

}

KisProofingConfigModel::KisProofingConfigModel(lager::cursor<KisProofingConfiguration> _data)
    : data(_data)
    , LAGER_QT(warningColor) {data[&KisProofingConfiguration::warningColor]}
    , LAGER_QT(proofingSpaceTuple) {
        lager::with(
            data[&KisProofingConfiguration::proofingModel],
            data[&KisProofingConfiguration::proofingDepth],
            data[&KisProofingConfiguration::proofingProfile]
        )
    }

    , LAGER_QT(conversionIntent) {data[&KisProofingConfiguration::conversionIntent]}
    , LAGER_QT(conversionIntentState) {
        lager::with(
            LAGER_QT(conversionIntent),
            lager::make_constant(true))
        .map(&calcIntentComboBoxState)
    }

    , LAGER_QT(convBlackPointCompensation) {data[&KisProofingConfiguration::useBlackPointCompensationFirstTransform]}

    , LAGER_QT(displayTransformMode) {data[&KisProofingConfiguration::displayMode]}
    , LAGER_QT(displayTransformModeState) {
        LAGER_QT(displayTransformMode)
        .map(&calcModeComboBoxState)
    }

    , LAGER_QT(enableCustomDisplayConfig) {
        LAGER_QT(displayTransformMode)
        .xform(kiszug::map_equal<int>(KisProofingConfiguration::Custom))}

    , LAGER_QT(displayIntent) {data[&KisProofingConfiguration::displayIntent]}
    , LAGER_QT(effectiveDisplayIntent) {
        lager::with(LAGER_QT(displayTransformMode),
                    LAGER_QT(displayIntent),
                    displayConfigOptions
                        .zoom(lager::lenses::first))
                .map(&calcEffectiveDisplayIntent)
    }
    , LAGER_QT(effectiveDisplayIntentState) {
        lager::with(
            LAGER_QT(effectiveDisplayIntent),
            LAGER_QT(enableCustomDisplayConfig))
        .map(&calcIntentComboBoxState)
    }

    , LAGER_QT(dispBlackPointCompensation) {data[&KisProofingConfiguration::displayFlags].zoom(conversionFlag(KoColorConversionTransformation::BlackpointCompensation))}
    , LAGER_QT(effectiveDispBlackPointCompensationState) {
        lager::with(LAGER_QT(displayTransformMode),
                    LAGER_QT(effectiveDisplayIntent),
                    LAGER_QT(dispBlackPointCompensation),
                    displayConfigOptions
                        .zoom(lager::lenses::second)
                        .zoom(conversionFlag(KoColorConversionTransformation::BlackpointCompensation)))
                .map(&calcEffectiveUseBPCState)
    }

    , LAGER_QT(adaptationSwitch) {
            data[&KisProofingConfiguration::displayFlags]
            .zoom(conversionFlag(KoColorConversionTransformation::NoAdaptationAbsoluteIntent))
            .zoom(kislager::lenses::logical_not())
    }
    , LAGER_QT(adaptationSwitchState) {
        lager::with(LAGER_QT(displayTransformMode),
                    LAGER_QT(displayIntent),
                    LAGER_QT(adaptationSwitch)).map(&calcEffectiveAdaptationSwitchState)
    }
{
    lager::watch(data, std::bind(&KisProofingConfigModel::modelChanged, this));
    lager::watch(displayConfigOptions, std::bind(&KisProofingConfigModel::modelChanged, this));
}

KisProofingConfigModel::~KisProofingConfigModel()
{
}

void KisProofingConfigModel::updateDisplayConfigOptions(KisDisplayConfig::Options options)
{
    if (displayConfigOptions.get() == options) return;
    displayConfigOptions.set(options);
}

void KisProofingConfigModel::setProofingColorSpaceIdAtomic(const QString &model, const QString &depth, const QString &profile)
{
    data.update([&] (KisProofingConfiguration config) {
        config.proofingModel = model;
        config.proofingDepth = depth;
        config.proofingProfile = profile;
        return config;
    });
}
