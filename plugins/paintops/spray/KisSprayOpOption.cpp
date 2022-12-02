/*
 *  SPDX-FileCopyrightText: 2022 Agata Cacko <cacko.azh@gmail.com>
 *  SPDX-FileCopyrightText: 2008-2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisSprayOpOption.h"
#include <KisPaintOpOptionUtils.h>

namespace kpou = KisPaintOpOptionUtils;

KisSprayOpOption::KisSprayOpOption(const KisPropertiesConfiguration *setting)
    : KisSprayOpOption(kpou::loadOptionData<KisSprayOpOptionData>(setting))
{
}

KisSprayOpOption::KisSprayOpOption(const KisSprayOpOptionData &_data)
{
	data = _data;
}


void KisSprayOpOption::updateDistributions()
{
    if (data.angularDistributionType == KisSprayOpOptionData::ParticleDistribution_CurveBased) {
        m_angularCurveBasedDistribution = KisSprayCurveBasedDistribution(data.angularDistributionCurve, data.angularDistributionCurveRepeat);
    }
    if (data.radialDistributionType == KisSprayOpOptionData::ParticleDistribution_Gaussian) {
        if (data.radialDistributionCenterBiased) {
            m_normalDistribution = KisSprayNormalDistribution(0.0, data.radialDistributionStdDeviation);
        } else {
            m_normalDistributionPolarDistance = KisSprayNormalDistributionPolarDistance(0.0, data.radialDistributionStdDeviation);
        }
    } else if (data.radialDistributionType == KisSprayOpOptionData::ParticleDistribution_ClusterBased) {
        m_clusterBasedDistributionPolarDistance = KisSprayClusterBasedDistributionPolarDistance(data.radialDistributionClusteringAmount);
    } else if (data.radialDistributionType == KisSprayOpOptionData::ParticleDistribution_CurveBased) {
        m_radialCurveBasedDistributionPolarDistance = KisSprayCurveBasedDistributionPolarDistance(data.radialDistributionCurve, data.radialDistributionCurveRepeat);
    }
}
