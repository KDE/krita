/*
 *  SPDX-FileCopyrightText: 2022 Agata Cacko <cacko.azh@gmail.com>
 *  SPDX-FileCopyrightText: 2008-2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisSprayOpOptionData.h"

#include "kis_properties_configuration.h"

bool KisSprayOpOptionData::read(const KisPropertiesConfiguration *settings)
{
    QVariant v;
    diameter = settings->getInt(SPRAY_DIAMETER);
    aspect = settings->getDouble(SPRAY_ASPECT);
    brushRotation = settings->getDouble(SPRAY_ROTATION);
    scale = settings->getDouble(SPRAY_SCALE);
    spacing = settings->getDouble(SPRAY_SPACING);
    jitterMovement = settings->getBool(SPRAY_JITTER_MOVEMENT);
    jitterAmount = settings->getDouble(SPRAY_JITTER_MOVE_AMOUNT);
    particleCount = settings->getDouble(SPRAY_PARTICLE_COUNT);
    coverage = settings->getDouble(SPRAY_COVERAGE) / 100.0;
    useDensity = settings->getBool(SPRAY_USE_DENSITY);
    {
        const QString angularDistributionTypeStr = settings->getString(SPRAY_ANGULAR_DISTRIBUTION_TYPE, "uniform");
        if (angularDistributionTypeStr == "curveBased") {
            angularDistributionType = ParticleDistribution_CurveBased;
        } else {
            angularDistributionType = ParticleDistribution_Uniform;
        }
        angularDistributionCurve = settings->getString(SPRAY_ANGULAR_DISTRIBUTION_CURVE, DEFAULT_CURVE_STRING);
        angularDistributionCurveRepeat = settings->getInt(SPRAY_ANGULAR_DISTRIBUTION_CURVE_REPEAT, 1);
    }
    {
        const QString radialDistributionTypeStr = settings->getString(SPRAY_RADIAL_DISTRIBUTION_TYPE, "");
        if (radialDistributionTypeStr == "uniform") {
            radialDistributionType = ParticleDistribution_Uniform;
        } else if (radialDistributionTypeStr == "gaussian") {
            radialDistributionType = ParticleDistribution_Gaussian;
        } else if (radialDistributionTypeStr == "clusterBased") {
            radialDistributionType = ParticleDistribution_ClusterBased;
        } else if (radialDistributionTypeStr == "curveBased") {
            radialDistributionType = ParticleDistribution_CurveBased;
        } else {
            // Old brush
            if (settings->getBool(SPRAY_GAUSS_DISTRIBUTION, false)) {
                radialDistributionType = ParticleDistribution_Gaussian;
            } else {
                radialDistributionType = ParticleDistribution_Uniform;
            }
        }
        radialDistributionStdDeviation = settings->getDouble(SPRAY_RADIAL_DISTRIBUTION_STD_DEVIATION, 0.5);
        radialDistributionClusteringAmount = settings->getDouble(SPRAY_RADIAL_DISTRIBUTION_CLUSTERING_AMOUNT, 0.0);
        radialDistributionCurve = settings->getString(SPRAY_RADIAL_DISTRIBUTION_CURVE, DEFAULT_CURVE_STRING);
        radialDistributionCurveRepeat = settings->getInt(SPRAY_RADIAL_DISTRIBUTION_CURVE_REPEAT, 1);
        radialDistributionCenterBiased = settings->getBool(SPRAY_RADIAL_DISTRIBUTION_CENTER_BIASED, true);
    }
    return true;
}

void KisSprayOpOptionData::write(KisPropertiesConfiguration *setting) const
{
    setting->setProperty(SPRAY_DIAMETER, diameter);
    setting->setProperty(SPRAY_ASPECT, aspect);
    setting->setProperty(SPRAY_ROTATION, brushRotation);
    setting->setProperty(SPRAY_SCALE, scale);
    setting->setProperty(SPRAY_SPACING, spacing);
    setting->setProperty(SPRAY_JITTER_MOVEMENT, jitterMovement);
    setting->setProperty(SPRAY_JITTER_MOVE_AMOUNT, jitterAmount);
    setting->setProperty(SPRAY_PARTICLE_COUNT, particleCount);
    setting->setProperty(SPRAY_COVERAGE, coverage * 100.0);
    setting->setProperty(SPRAY_USE_DENSITY, useDensity);
    if (angularDistributionType == ParticleDistribution_CurveBased) {
        setting->setProperty(SPRAY_ANGULAR_DISTRIBUTION_TYPE, "curveBased");
    } else {
        setting->setProperty(SPRAY_ANGULAR_DISTRIBUTION_TYPE, "uniform");
    }
    if (radialDistributionType == ParticleDistribution_Gaussian) {
        setting->setProperty(SPRAY_GAUSS_DISTRIBUTION, true);
        setting->setProperty(SPRAY_RADIAL_DISTRIBUTION_TYPE, "gaussian");
    } else if (radialDistributionType == ParticleDistribution_ClusterBased) {
        setting->setProperty(SPRAY_RADIAL_DISTRIBUTION_TYPE, "clusterBased");
    } else if (radialDistributionType == ParticleDistribution_CurveBased) {
        setting->setProperty(SPRAY_RADIAL_DISTRIBUTION_TYPE, "curveBased");
    } else {
        setting->setProperty(SPRAY_RADIAL_DISTRIBUTION_TYPE, "uniform");
    }
    QVariant c;
    c.setValue(angularDistributionCurve);
    setting->setProperty(SPRAY_ANGULAR_DISTRIBUTION_CURVE, c);
    setting->setProperty(SPRAY_ANGULAR_DISTRIBUTION_CURVE_REPEAT, angularDistributionCurveRepeat);
    setting->setProperty(SPRAY_RADIAL_DISTRIBUTION_STD_DEVIATION, radialDistributionStdDeviation);
    setting->setProperty(SPRAY_RADIAL_DISTRIBUTION_CLUSTERING_AMOUNT, radialDistributionClusteringAmount);
    c.setValue(radialDistributionCurve);
    setting->setProperty(SPRAY_RADIAL_DISTRIBUTION_CURVE, c);
    setting->setProperty(SPRAY_RADIAL_DISTRIBUTION_CURVE_REPEAT, radialDistributionCurveRepeat);
    setting->setProperty(SPRAY_GAUSS_DISTRIBUTION, radialDistributionType == ParticleDistribution_Gaussian);
    setting->setProperty(SPRAY_RADIAL_DISTRIBUTION_CENTER_BIASED, radialDistributionCenterBiased);
}

