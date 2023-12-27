/*
 *  SPDX-FileCopyrightText: 2022 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_SPRAYOP_OPTION_H
#define KIS_SPRAYOP_OPTION_H

#include "KisSprayRandomDistributions.h"
#include "KisSprayOpOptionData.h"

class KisPropertiesConfiguration;



class KisSprayOpOption
{
public:
	KisSprayOpOption(const KisPropertiesConfiguration *setting);
	KisSprayOpOption(const KisSprayOpOptionData &data);
	
	
public:
	// those are just functors, no data inside
    KisSprayUniformDistribution m_uniformDistribution {};
    KisSprayCurveBasedDistribution m_angularCurveBasedDistribution {};
    KisSprayUniformDistributionPolarDistance m_uniformDistributionPolarDistance {};
    KisSprayNormalDistribution m_normalDistribution {};
    KisSprayNormalDistributionPolarDistance m_normalDistributionPolarDistance {};
    KisSprayClusterBasedDistributionPolarDistance m_clusterBasedDistributionPolarDistance {};
    KisSprayCurveBasedDistributionPolarDistance m_radialCurveBasedDistributionPolarDistance {};
	
	KisSprayOpOptionData data;
	
	void updateDistributions();
};

#endif // KIS_SPRAYOP_OPTION_H
