/*
 *  SPDX-FileCopyrightText: 2022 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisSprayOpOptionModel.h"

#include <KisLager.h>


namespace {

auto angularDistributionTypeToInt = lager::lenses::getset(
[] (const KisSprayOpOptionData::ParticleDistribution source) -> int
{
	if (source == KisSprayOpOptionData::ParticleDistribution_Uniform) {
		return 0;
	} else { // source == KisSprayOpOptionData::ParticleDistribution_CurveBased
		return 1;
	}
},
[] (KisSprayOpOptionData::ParticleDistribution dst, int currentValue)
{
	if (currentValue == 0) {
		dst = KisSprayOpOptionData::ParticleDistribution_Uniform;
	} else { // == 1
		dst = KisSprayOpOptionData::ParticleDistribution_CurveBased;
	}

    return dst;
}
);

}


KisSprayOpOptionModel::KisSprayOpOptionModel(lager::cursor<KisSprayOpOptionData> _optionData)
    : optionData(_optionData)
    , LAGER_QT(diameter) {_optionData[&KisSprayOpOptionData::diameter].zoom(kislager::lenses::do_static_cast<quint16, int>)}
    , LAGER_QT(aspect) {_optionData[&KisSprayOpOptionData::aspect]}
    , LAGER_QT(brushRotation) {_optionData[&KisSprayOpOptionData::brushRotation]}
    , LAGER_QT(scale) {_optionData[&KisSprayOpOptionData::scale]}
    , LAGER_QT(spacing) {_optionData[&KisSprayOpOptionData::spacing]}
    , LAGER_QT(jitterMovement) {_optionData[&KisSprayOpOptionData::jitterMovement]}
    , LAGER_QT(jitterAmount) {_optionData[&KisSprayOpOptionData::jitterAmount]}
    , LAGER_QT(useDensity) {_optionData[&KisSprayOpOptionData::useDensity]}
    , LAGER_QT(isNumParticlesVisible) {LAGER_QT(useDensity).map(std::logical_not<>{})}
    , LAGER_QT(particleCount) {_optionData[&KisSprayOpOptionData::particleCount].zoom(kislager::lenses::do_static_cast<quint16, int>)}
    , LAGER_QT(coverage) {_optionData[&KisSprayOpOptionData::coverage]}
    
    , LAGER_QT(angularDistributionType) {_optionData[&KisSprayOpOptionData::angularDistributionType]
		.zoom(angularDistributionTypeToInt)}
    , LAGER_QT(angularDistributionCurve) {_optionData[&KisSprayOpOptionData::angularDistributionCurve]}
    , LAGER_QT(angularDistributionCurveRepeat) {_optionData[&KisSprayOpOptionData::angularDistributionCurveRepeat]}
    , LAGER_QT(radialDistributionType) {_optionData[&KisSprayOpOptionData::radialDistributionType]
		.zoom(kislager::lenses::do_static_cast<KisSprayOpOptionData::ParticleDistribution, int>)}
    , LAGER_QT(radialDistributionStdDeviation) {_optionData[&KisSprayOpOptionData::radialDistributionStdDeviation]}
    , LAGER_QT(radialDistributionClusteringAmount) {_optionData[&KisSprayOpOptionData::radialDistributionClusteringAmount]}
    , LAGER_QT(radialDistributionCurve) {_optionData[&KisSprayOpOptionData::radialDistributionCurve]}
    , LAGER_QT(radialDistributionCurveRepeat) {_optionData[&KisSprayOpOptionData::radialDistributionCurveRepeat]}
    , LAGER_QT(radialDistributionCenterBiased) {_optionData[&KisSprayOpOptionData::radialDistributionCenterBiased]}
    
{
}
