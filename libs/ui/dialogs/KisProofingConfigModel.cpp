/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisProofingConfigModel.h"
#include <KisLager.h>

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

auto displayState = lager::lenses::getset(
    [] (const KisProofingConfiguration &conf) -> KisProofingConfiguration::DisplayTransformState {
        return conf.displayMode;
    },
    [] (KisProofingConfiguration conf, const KisProofingConfiguration::DisplayTransformState &state) -> KisProofingConfiguration {
        conf.displayMode = state;

        // TODO: Use kisconfig to get Monitor flags somehow?
        conf.displayIntent = conf.determineDisplayIntent(KoColorConversionTransformation::IntentRelativeColorimetric);
        conf.displayFlags = conf.determineDisplayFlags(KoColorConversionTransformation::BlackpointCompensation);
        conf.adaptationState = conf.determineAdaptationState();

        return conf;
    }
    );
}
KisProofingConfigModel::KisProofingConfigModel(lager::cursor<KisProofingConfiguration> _data)
    : data(_data)
    , LAGER_QT(warningColor) {data[&KisProofingConfiguration::warningColor]}
    , LAGER_QT(proofingProfile) {data[&KisProofingConfiguration::proofingProfile]}
    , LAGER_QT(proofingModel) {data[&KisProofingConfiguration::proofingModel]}
    , LAGER_QT(proofingDepth) {data[&KisProofingConfiguration::proofingDepth]}
    , LAGER_QT(storeSoftproofingInsideImage) {data[&KisProofingConfiguration::storeSoftproofingInsideImage]}
    , LAGER_QT(conversionIntent) {data[&KisProofingConfiguration::conversionIntent]}
    , LAGER_QT(convBlackPointCompensation) {data[&KisProofingConfiguration::useBlackPointCompensationFirstTransform]}
    , LAGER_QT(displayTransformState) {data.zoom(displayState)}
    , LAGER_QT(displayIntent) {data[&KisProofingConfiguration::displayIntent]}
    , LAGER_QT(dispBlackPointCompensation) {data[&KisProofingConfiguration::displayFlags].zoom(conversionFlag(KoColorConversionTransformation::BlackpointCompensation))}
    , LAGER_QT(adaptationState) {data[&KisProofingConfiguration::adaptationState].zoom(kislager::lenses::scale_real_to_int(ADAPTATION_MULTIPLIER))}
    , LAGER_QT(adaptationRangeMax) {ADAPTATION_MULTIPLIER}

{
    lager::watch(data, std::bind(&KisProofingConfigModel::modelChanged, this));
}
