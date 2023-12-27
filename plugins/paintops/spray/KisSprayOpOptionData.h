/*
 *  SPDX-FileCopyrightText: 2022 Agata Cacko <cacko.azh@gmail.com>
 *  SPDX-FileCopyrightText: 2008-2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_SPRAYOP_OPTION_DATA_H
#define KIS_SPRAYOP_OPTION_DATA_H


#include "kis_types.h"
#include <boost/operators.hpp>
#include <kritapaintop_export.h>

#include <kis_cubic_curve.h>

#include "KisSprayRandomDistributions.h"

class KisPropertiesConfiguration;


const QString SPRAY_DIAMETER = "Spray/diameter";
const QString SPRAY_ASPECT = "Spray/aspect";
const QString SPRAY_ROTATION = "Spray/rotation";
const QString SPRAY_SCALE = "Spray/scale";
const QString SPRAY_SPACING = "Spray/spacing";
const QString SPRAY_JITTER_MOVEMENT = "Spray/jitterMovement";
const QString SPRAY_JITTER_MOVE_AMOUNT = "Spray/jitterMoveAmount";
const QString SPRAY_USE_DENSITY = "Spray/useDensity";
const QString SPRAY_PARTICLE_COUNT = "Spray/particleCount";
const QString SPRAY_COVERAGE = "Spray/coverage";
const QString SPRAY_ANGULAR_DISTRIBUTION_TYPE = "Spray/angularDistributionType";
const QString SPRAY_ANGULAR_DISTRIBUTION_CURVE = "Spray/angularDistributionCurve";
const QString SPRAY_ANGULAR_DISTRIBUTION_CURVE_REPEAT = "Spray/angularDistributionCurveRepeat";
const QString SPRAY_RADIAL_DISTRIBUTION_TYPE = "Spray/radialDistributionType";
const QString SPRAY_RADIAL_DISTRIBUTION_STD_DEVIATION = "Spray/radialDistributionStdDeviation";
const QString SPRAY_RADIAL_DISTRIBUTION_CLUSTERING_AMOUNT = "Spray/radialDistributionClusteringAmount";
const QString SPRAY_RADIAL_DISTRIBUTION_CURVE = "Spray/radialDistributionCurve";
const QString SPRAY_RADIAL_DISTRIBUTION_CURVE_REPEAT = "Spray/radialDistributionCurveRepeat";
const QString SPRAY_RADIAL_DISTRIBUTION_CENTER_BIASED = "Spray/radialDistributionCenterBiased";
const QString SPRAY_GAUSS_DISTRIBUTION = "Spray/gaussianDistribution";


struct KisSprayOpOptionData : boost::equality_comparable<KisSprayOpOptionData>
{
	enum ParticleDistribution
    {
        ParticleDistribution_Uniform,
        ParticleDistribution_Gaussian,
        ParticleDistribution_ClusterBased,
        ParticleDistribution_CurveBased
    };
	
    inline friend bool operator==(const KisSprayOpOptionData &lhs, const KisSprayOpOptionData &rhs) {
        return lhs.diameter == rhs.diameter // 10 entries
			&& lhs.aspect == rhs.aspect
			&& lhs.brushRotation == rhs.brushRotation
			&& lhs.scale == rhs.scale
			&& lhs.spacing == rhs.spacing
			&& lhs.jitterMovement == rhs.jitterMovement
			&& lhs.jitterAmount == rhs.jitterAmount
			&& lhs.useDensity == rhs.useDensity
			&& lhs.particleCount == rhs.particleCount
			&& lhs.coverage == rhs.coverage
			// 9 entries
			&& lhs.angularDistributionType == rhs.angularDistributionType
			&& lhs.angularDistributionCurve == rhs.angularDistributionCurve
			&& lhs.angularDistributionCurveRepeat == rhs.angularDistributionCurveRepeat
			&& lhs.radialDistributionType == rhs.radialDistributionType
			&& lhs.radialDistributionStdDeviation == rhs.radialDistributionStdDeviation
			&& lhs.radialDistributionClusteringAmount == rhs.radialDistributionClusteringAmount
			&& lhs.radialDistributionCurve == rhs.radialDistributionCurve
			&& lhs.radialDistributionCurveRepeat == rhs.radialDistributionCurveRepeat
			&& lhs.radialDistributionCenterBiased == rhs.radialDistributionCenterBiased;
			// 7 entries - but there is no need to compare functors
			
    }

	// sane defaults (for Coverity)
	// NOTE: if you add any new variable, make sure it's present in all places! including == function
	// 10 entries
    quint16 diameter {100};
    qreal aspect {1.0};
    qreal brushRotation {0.0};
    qreal scale {1.0};
    qreal spacing {0.5};
    bool jitterMovement {false};
    qreal jitterAmount {1.0};
    bool useDensity {false};
    quint16 particleCount {12};
    qreal coverage {0.003};
	
    // 9 entries
    ParticleDistribution angularDistributionType {ParticleDistribution_Uniform};
    QString angularDistributionCurve {DEFAULT_CURVE_STRING};
    int angularDistributionCurveRepeat {1};
    ParticleDistribution radialDistributionType {ParticleDistribution_Uniform};
    qreal radialDistributionStdDeviation {0.5};
    qreal radialDistributionClusteringAmount {0.0};
    QString radialDistributionCurve {DEFAULT_CURVE_STRING};
    int radialDistributionCurveRepeat {1};
    bool radialDistributionCenterBiased {false};

	// functions
    bool read(const KisPropertiesConfiguration *setting);
    void write(KisPropertiesConfiguration *setting) const;
};

#endif // KIS_SPRAYOP_OPTION_DATA_H
