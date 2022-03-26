/*
 *  SPDX-FileCopyrightText: 2008, 2009, 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_SPRAYOP_OPTION_H
#define KIS_SPRAYOP_OPTION_H

#include <kis_paintop_option.h>
#include <kis_cubic_curve.h>

#include "KisSprayRandomDistributions.h"

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

class KisSprayOpOptionsWidget;

class KisSprayOpOption : public KisPaintOpOption
{
    Q_OBJECT
    
public:
    KisSprayOpOption();
    ~KisSprayOpOption() override;

    void setDiameter(int diameter) const;
    int diameter() const;

    qreal brushAspect() const;

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

private:
    KisSprayOpOptionsWidget *m_options;

private Q_SLOTS:
    void slot_angularDistCombo_currentIndexChanged(int index);
    void slot_radialDistCombo_currentIndexChanged(int index);
};

class KisSprayOptionProperties : public KisPaintopPropertiesBase
{
public:
    enum ParticleDistribution
    {
        ParticleDistribution_Uniform,
        ParticleDistribution_Gaussian,
        ParticleDistribution_ClusterBased,
        ParticleDistribution_CurveBased
    };

    quint16 diameter() const;
    int radius() const;
    qreal aspect() const;
    qreal brushRotation() const;
    qreal scale() const;
    qreal spacing() const;
    bool jitterMovement() const;
    qreal jitterAmount() const;
    bool useDensity() const;
    quint16 particleCount() const;
    qreal coverage() const;
    ParticleDistribution angularDistributionType() const;
    KisCubicCurve angularDistributionCurve() const;
    int angularDistributionCurveRepeat() const;
    ParticleDistribution radialDistributionType() const;
    qreal radialDistributionStdDeviation() const;
    qreal radialDistributionClusteringAmount() const;
    KisCubicCurve radialDistributionCurve() const;
    int radialDistributionCurveRepeat() const;
    bool radialDistributionCenterBiased() const;

    void setDiameter(quint16 newDiameter);
    void setAspect(qreal newAspect);
    void setBrushRotation(qreal newBrushRotation);
    void setScale(qreal newScale);
    void setSpacing(qreal newSpacing);
    void setJitterMovement(bool newJitterMovement);
    void setJitterAmount(qreal newJitterAmount);
    void setUseDensity(bool newUseDensity);
    void setParticleCount(quint16 newParticleCount);
    void setCoverage(qreal newCoverage);
    void setAngularDistributionType(ParticleDistribution newAngularDistributionType);
    void setAngularDistributionCurve(KisCubicCurve newAngularDistributionCurve);
    void setAngularDistributionCurveRepeat(int newAngularDistributionCurveRepeat);
    void setRadialDistributionType(ParticleDistribution newRadialDistributionType);
    void setRadialDistributionStdDeviation(qreal newRadialDistributionStdDeviation);
    void setRadialDistributionClusteringAmount(qreal newRadialDistributionClusteringAmount);
    void setRadialDistributionCurve(KisCubicCurve newRadialDistributionCurve);
    void setRadialDistributionCurveRepeat(int newRadialDistributionCurveRepeat);
    void setRadialDistributionCenterBiased(bool newRadialDistributionCenterBiased);

    const KisSprayUniformDistribution& uniformDistribution() const;
    const KisSprayCurveBasedDistribution& angularCurveBasedDistribution() const;
    const KisSprayUniformDistributionPolarDistance& uniformDistributionPolarDistance() const;
    const KisSprayNormalDistribution& normalDistribution() const;
    const KisSprayNormalDistributionPolarDistance& normalDistributionPolarDistance() const;
    const KisSprayClusterBasedDistribution& clusterBasedDistribution() const;
    const KisSprayClusterBasedDistributionPolarDistance& clusterBasedDistributionPolarDistance() const;
    const KisSprayCurveBasedDistribution& radialCurveBasedDistribution() const;
    const KisSprayCurveBasedDistributionPolarDistance& radialCurveBasedDistributionPolarDistance() const;
    void updateDistributions();

    void readOptionSettingImpl(const KisPropertiesConfiguration *settings) override;
    void writeOptionSettingImpl(KisPropertiesConfiguration *setting) const override;

private:
    // sane defaults (for Coverity)
    quint16 m_diameter {100};
    qreal m_aspect {1.0};
    qreal m_brushRotation {0.0};
    qreal m_scale {1.0};
    qreal m_spacing {0.5};
    bool m_jitterMovement {false};
    qreal m_jitterAmount {1.0};
    bool m_useDensity {false};
    quint16 m_particleCount {12};
    qreal m_coverage {0.003};
    ParticleDistribution m_angularDistributionType {ParticleDistribution_Uniform};
    KisCubicCurve m_angularDistributionCurve {QList<QPointF>{{0.0, 1.0}, {1.0, 0.0}}};
    int m_angularDistributionCurveRepeat {1};
    ParticleDistribution m_radialDistributionType {ParticleDistribution_Uniform};
    qreal m_radialDistributionStdDeviation {0.5};
    qreal m_radialDistributionClusteringAmount {0.0};
    KisCubicCurve m_radialDistributionCurve {QList<QPointF>{{0.0, 1.0}, {1.0, 0.0}}};
    int m_radialDistributionCurveRepeat {1};
    bool m_radialDistributionCenterBiased {false};

    KisSprayUniformDistribution m_uniformDistribution {};
    KisSprayCurveBasedDistribution m_angularCurveBasedDistribution {};
    KisSprayUniformDistributionPolarDistance m_uniformDistributionPolarDistance {};
    KisSprayNormalDistribution m_normalDistribution {};
    KisSprayNormalDistributionPolarDistance m_normalDistributionPolarDistance {};
    KisSprayClusterBasedDistributionPolarDistance m_clusterBasedDistributionPolarDistance {};
    KisSprayCurveBasedDistributionPolarDistance m_radialCurveBasedDistributionPolarDistance {};
};

#endif
