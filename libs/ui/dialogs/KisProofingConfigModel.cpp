/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisProofingConfigModel.h"
#include <KisLager.h>
#include <KisZug.h>

#include <kis_display_color_converter.h>
#include <lager/lenses/tuple.hpp>

const int ADAPTATION_MULTIPLIER = 20;

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

bool calcEffectiveUseBPC(KisProofingConfiguration::DisplayTransformState displayMode,
                        bool localUseBPC,
                        bool globalUseBPC)
{
    return displayMode == KisProofingConfiguration::Monitor ? globalUseBPC:
                                                              displayMode == KisProofingConfiguration::Paper ?
                                                                  false: localUseBPC;
}

int calcEffectiveAdaptation(KisProofingConfiguration::DisplayTransformState displayMode,
                        int localAdaptation)
{
    return displayMode == KisProofingConfiguration::Monitor ? ADAPTATION_MULTIPLIER:
                                                              displayMode == KisProofingConfiguration::Paper ?
                                                                  0: localAdaptation;
}

}

KisProofingConfigModel::KisProofingConfigModel(lager::cursor<KisProofingConfiguration> _data)
    : data(_data)
    , displayConfigOptionsCursor(lager::make_sensor([&]{return m_displayConfigOptions;}))
    , LAGER_QT(warningColor) {data[&KisProofingConfiguration::warningColor]}
    , LAGER_QT(proofingProfile) {data[&KisProofingConfiguration::proofingProfile]}
    , LAGER_QT(proofingModel) {data[&KisProofingConfiguration::proofingModel]}
    , LAGER_QT(proofingDepth) {data[&KisProofingConfiguration::proofingDepth]}
    , LAGER_QT(storeSoftproofingInsideImage) {data[&KisProofingConfiguration::storeSoftproofingInsideImage]}
    , LAGER_QT(conversionIntent) {data[&KisProofingConfiguration::conversionIntent]}
    , LAGER_QT(convBlackPointCompensation) {data[&KisProofingConfiguration::useBlackPointCompensationFirstTransform]}
    , LAGER_QT(displayTransformState) {data[&KisProofingConfiguration::displayMode]}
    , LAGER_QT(displayIntent) {data[&KisProofingConfiguration::displayIntent]}
    , LAGER_QT(dispBlackPointCompensation) {data[&KisProofingConfiguration::displayFlags].zoom(conversionFlag(KoColorConversionTransformation::BlackpointCompensation))}

    , LAGER_QT(effectiveDisplayIntent) {
        lager::with(LAGER_QT(displayTransformState),
                    LAGER_QT(displayIntent),
                    displayConfigOptionsCursor
                        .zoom(lager::lenses::first))
                .map(&calcEffectiveDisplayIntent)}

    , LAGER_QT(effectiveDispBlackPointCompensation) {
        lager::with(LAGER_QT(displayTransformState),
                    LAGER_QT(dispBlackPointCompensation),
                    displayConfigOptionsCursor
                        .zoom(lager::lenses::second)
                        .zoom(conversionFlag(KoColorConversionTransformation::BlackpointCompensation)))
                .map(&calcEffectiveUseBPC)}
    , LAGER_QT(adaptationState) {data[&KisProofingConfiguration::adaptationState].zoom(kislager::lenses::scale_real_to_int(ADAPTATION_MULTIPLIER))}
    , LAGER_QT(adaptationRangeMax) {ADAPTATION_MULTIPLIER}
    , LAGER_QT(effectiveAdaptationState) {
    lager::with(LAGER_QT(displayTransformState),
                LAGER_QT(adaptationState))
            .map(&calcEffectiveAdaptation)}
    , LAGER_QT(enableDisplayToggles) {LAGER_QT(displayTransformState) .xform(kiszug::map_equal<int>(KisProofingConfiguration::Custom))}
    , LAGER_QT(enableAdaptationSlider) {LAGER_QT(displayIntent).xform(kiszug::map_equal<int>(KoColorConversionTransformation::IntentAbsoluteColorimetric))}
    , LAGER_QT(enableDisplayBlackPointCompensation) {LAGER_QT(displayIntent).xform(kiszug::map_not_equal<int>(KoColorConversionTransformation::IntentAbsoluteColorimetric))}
{
    lager::watch(data, std::bind(&KisProofingConfigModel::modelChanged, this));
    lager::watch(displayConfigOptionsCursor, std::bind(&KisProofingConfigModel::modelChanged, this));
}

KisProofingConfigModel::~KisProofingConfigModel()
{
}

void KisProofingConfigModel::updateDisplayConfigOptions(KisDisplayConfig::Options options)
{
    if (m_displayConfigOptions == options) return;
    m_displayConfigOptions = options;
    lager::commit(displayConfigOptionsCursor);
}
