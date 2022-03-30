/*
 *  SPDX-FileCopyrightText: 2008, 2009, 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "kis_sprayop_option.h"
#include <klocalizedstring.h>
#include <kis_signals_blocker.h>

#include "ui_wdgsprayoptions.h"

class KisSprayOpOptionsWidget: public QWidget, public Ui::WdgSprayOptions
{
public:
    KisSprayOpOptionsWidget(QWidget *parent = 0)
        : QWidget(parent) {
        setupUi(this);
    }
};

KisSprayOpOption::KisSprayOpOption()
    : KisPaintOpOption(i18n("Spray Area"), KisPaintOpOption::GENERAL, false)
{
    setObjectName("KisSprayOpOption");

    m_checkable = false;
    m_options = new KisSprayOpOptionsWidget();

    m_options->diameterSpinBox->setRange(1, 1000, 0);
    m_options->diameterSpinBox->setValue(100);
    m_options->diameterSpinBox->setExponentRatio(1.5);
    m_options->diameterSpinBox->setSuffix(i18n(" px"));

    m_options->aspectSPBox->setRange(0.0, 2.0, 2);
    m_options->aspectSPBox->setSingleStep(0.01);
    m_options->aspectSPBox->setValue(1.0);

    m_options->rotationAngleSelector->setDecimals(0);
    m_options->rotationAngleSelector->setIncreasingDirection(KisAngleGauge::IncreasingDirection_Clockwise);

    m_options->scaleSpin->setRange(0.0, 10.0, 2);
    m_options->scaleSpin->setSingleStep(0.01);
    m_options->scaleSpin->setValue(1.0);

    m_options->spacingSpin->setRange(0.0, 5.0, 2);
    m_options->spacingSpin->setSingleStep(0.01);
    m_options->spacingSpin->setValue(0.5);

    m_options->jitterMovementSpin->setRange(0.0,5.0, 1);
    m_options->jitterMovementSpin->setSingleStep(0.1);
    m_options->jitterMovementSpin->setValue(1.0);

    m_options->particlesSpinBox->setRange(1.0, 1000.0, 0);
    m_options->particlesSpinBox->setValue(12);
    m_options->particlesSpinBox->setExponentRatio(3.0);

    m_options->coverageSpin->setRange(0.001, 0.02, 3);
    m_options->coverageSpin->setSingleStep(0.001);
    m_options->coverageSpin->setValue(0.003);
    m_options->coverageSpin->setSuffix(i18n("%"));
    m_options->coverageSpin->setVisible(false);
    
    m_options->angularDistCombo->setToolTip(i18n("Select how the particles are distributed as a function of the angle to the center of the spray area."));

    m_options->curveAngularDistWidget->setToolTip(i18n(
        "Set a custom distribution of the particles."
        "\nThe horizontal axis represents the angle from 0 to 360 degrees."
        "\nThe vertical axis represents how probable it is for a particle ending at that angle."
        "\nThe higher the curve at a given angle, the more particles will end at that angle."
    ));
    m_options->curveAngularDistSpin->setPrefix(i18n("Repeat: "));
    m_options->curveAngularDistSpin->setSuffix(i18nc("Times symbol, like in 10x", "x"));
    m_options->curveAngularDistSpin->setRange(1, 10);
    m_options->curveAngularDistSpin->setToolTip(i18n(
        "Set how many times should the curve repeat from 0 degrees to 360 degrees."
    ));

    m_options->radialDistCombo->setToolTip(i18n("Select how the particles are distributed as a function of the distance from the center of the spray area."));


    m_options->centerBiasedPolarDistanceBox->setToolTip(i18n("Activates the old behavior where the particles are more accumulated towards the center of the spray area."));

    m_options->stdDeviationRadialDistSpin->setPrefix(i18n("Standard deviation: "));
    m_options->stdDeviationRadialDistSpin->setRange(0.01, 1.0, 2);
    m_options->stdDeviationRadialDistSpin->setSingleStep(0.01);
    m_options->stdDeviationRadialDistSpin->setToolTip(i18n(
        "Set the standard deviation for the gaussian distribution."
        "\nLower values will make the particles concentrate towards the center of the spray area."
    ));

    m_options->clusterRadialDistSpin->setPrefix(i18n("Clustering amount: "));
    m_options->clusterRadialDistSpin->setRange(-100.0, 100.0, 2);
    m_options->clusterRadialDistSpin->setSoftRange(-10.0, 10.0);
    m_options->clusterRadialDistSpin->setToolTip(i18n(
        "Set how the particles should spread in the spray area."
        "\nPositive values will make the particles concentrate towards the center of the spray area."
        "\nNegative values will make the particles concentrate towards the border of the spray area."
        "\nValues near 0 will make the particles spread more uniformly."
    ));

    m_options->curveRadialDistWidget->setToolTip(i18n(
        "Set a custom distribution of the particles."
        "\nThe horizontal axis represents the distance from the center to the border of the spray area."
        "\nThe vertical axis represents how probable it is for a particle ending at that distance."
        "\nThe higher the curve at a given distance, the more particles will end at that distance."
    ));
    m_options->curveRadialDistSpin->setPrefix(i18n("Repeat: "));
    m_options->curveRadialDistSpin->setSuffix(i18nc("Times symbol, like in 10x", "x"));
    m_options->curveRadialDistSpin->setRange(1, 10);
    m_options->curveRadialDistSpin->setToolTip(i18n(
        "Set how many times should the curve repeat from the center to the border of the spray area."
    ));

    m_options->layoutAngularDist->takeAt(1);
    m_options->curveAngularDistContainer->setVisible(false);
    m_options->layoutRadialDist->takeAt(1);
    m_options->layoutRadialDist->takeAt(1);
    m_options->layoutRadialDist->takeAt(1);
    m_options->stdDeviationRadialDistSpin->setVisible(false);
    m_options->clusterRadialDistSpin->setVisible(false);
    m_options->curveRadialDistContainer->setVisible(false);

    connect(m_options->diameterSpinBox, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->aspectSPBox, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->rotationAngleSelector, SIGNAL(angleChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->scaleSpin, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->spacingSpin, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->jitterMoveBox, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    connect(m_options->jitterMovementSpin, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->countRadioButton, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    connect(m_options->densityRadioButton, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    connect(m_options->particlesSpinBox, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->coverageSpin, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->angularDistCombo, SIGNAL(currentIndexChanged(int)), SLOT(emitSettingChanged()));
    connect(m_options->curveAngularDistWidget, SIGNAL(modified()), SLOT(emitSettingChanged()));
    connect(m_options->curveAngularDistSpin, SIGNAL(valueChanged(int)), SLOT(emitSettingChanged()));
    connect(m_options->radialDistCombo, SIGNAL(currentIndexChanged(int)), SLOT(emitSettingChanged()));
    connect(m_options->stdDeviationRadialDistSpin, SIGNAL(valueChanged(double)), SLOT(emitSettingChanged()));
    connect(m_options->clusterRadialDistSpin, SIGNAL(valueChanged(double)), SLOT(emitSettingChanged()));
    connect(m_options->curveRadialDistWidget, SIGNAL(modified()), SLOT(emitSettingChanged()));
    connect(m_options->curveRadialDistSpin, SIGNAL(valueChanged(int)), SLOT(emitSettingChanged()));
    connect(m_options->centerBiasedPolarDistanceBox, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));

    connect(m_options->jitterMoveBox, SIGNAL(toggled(bool)), m_options->jitterMovementSpin, SLOT(setEnabled(bool)));
    connect(m_options->countRadioButton, SIGNAL(toggled(bool)), m_options->particlesSpinBox, SLOT(setVisible(bool)));
    connect(m_options->densityRadioButton, SIGNAL(toggled(bool)), m_options->coverageSpin, SLOT(setVisible(bool)));
    connect(m_options->angularDistCombo, SIGNAL(currentIndexChanged(int)), SLOT(slot_angularDistCombo_currentIndexChanged(int)));
    connect(m_options->radialDistCombo, SIGNAL(currentIndexChanged(int)), SLOT(slot_radialDistCombo_currentIndexChanged(int)));

    setConfigurationPage(m_options);
}

KisSprayOpOption::~KisSprayOpOption()
{
    delete m_options;
}

void KisSprayOpOption::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    KisSprayOptionProperties op;

    op.setDiameter(m_options->diameterSpinBox->value());
    op.setAspect(m_options->aspectSPBox->value());
    op.setBrushRotation(m_options->rotationAngleSelector->angle());
    op.setScale(m_options->scaleSpin->value());
    op.setSpacing(m_options->spacingSpin->value());
    op.setJitterMovement(m_options->jitterMoveBox->isChecked());
    op.setJitterAmount(m_options->jitterMovementSpin->value());
    op.setUseDensity(m_options->densityRadioButton->isChecked());
    op.setParticleCount(m_options->particlesSpinBox->value());
    op.setCoverage(m_options->coverageSpin->value());
    {
        if (m_options->angularDistCombo->currentIndex() == 0) {
            op.setAngularDistributionType(KisSprayOptionProperties::ParticleDistribution_Uniform);
        } else {
            op.setAngularDistributionType(KisSprayOptionProperties::ParticleDistribution_CurveBased);
        }
    }
    op.setAngularDistributionCurve(m_options->curveAngularDistWidget->curve());
    op.setAngularDistributionCurveRepeat(m_options->curveAngularDistSpin->value());
    {
        if (m_options->radialDistCombo->currentIndex() == 0) {
            op.setRadialDistributionType(KisSprayOptionProperties::ParticleDistribution_Uniform);
        } else if (m_options->radialDistCombo->currentIndex() == 1) {
            op.setRadialDistributionType(KisSprayOptionProperties::ParticleDistribution_Gaussian);
        } else if (m_options->radialDistCombo->currentIndex() == 2) {
            op.setRadialDistributionType(KisSprayOptionProperties::ParticleDistribution_ClusterBased);
        } else {
            op.setRadialDistributionType(KisSprayOptionProperties::ParticleDistribution_CurveBased);
        }
    }
    op.setRadialDistributionStdDeviation(m_options->stdDeviationRadialDistSpin->value());
    op.setRadialDistributionClusteringAmount(m_options->clusterRadialDistSpin->value());
    op.setRadialDistributionCurve(m_options->curveRadialDistWidget->curve());
    op.setRadialDistributionCurveRepeat(m_options->curveRadialDistSpin->value());
    op.setRadialDistributionCenterBiased(m_options->centerBiasedPolarDistanceBox->isChecked());

    op.writeOptionSetting(setting);
}

void KisSprayOpOption::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisSprayOptionProperties op;
    op.readOptionSetting(setting);

    KisSignalsBlocker blocker1(m_options->diameterSpinBox, m_options->aspectSPBox,
                               m_options->rotationAngleSelector, m_options->scaleSpin,
                               m_options->spacingSpin, m_options->jitterMovementSpin);
    KisSignalsBlocker blocker2(m_options->jitterMoveBox, m_options->densityRadioButton,
                               m_options->countRadioButton, m_options->particlesSpinBox,
                               m_options->coverageSpin, m_options->angularDistCombo);
    KisSignalsBlocker blocker3(m_options->curveAngularDistWidget, m_options->angularDistCombo,
                               m_options->stdDeviationRadialDistSpin, m_options->clusterRadialDistSpin,
                               m_options->curveRadialDistWidget, m_options->centerBiasedPolarDistanceBox);
    
    m_options->diameterSpinBox->setValue(op.diameter());
    m_options->aspectSPBox->setValue(op.aspect());
    m_options->rotationAngleSelector->setAngle(op.brushRotation());
    m_options->scaleSpin->setValue(op.scale());
    m_options->spacingSpin->setValue(op.spacing());
    m_options->jitterMoveBox->setChecked(op.jitterMovement());
    m_options->jitterMovementSpin->setValue(op.jitterAmount());
    m_options->densityRadioButton->setChecked(op.useDensity());
    m_options->countRadioButton->setChecked(!op.useDensity());
    m_options->particlesSpinBox->setValue(op.particleCount());
    m_options->coverageSpin->setValue(op.coverage());
    {
        const int index = op.angularDistributionType() == KisSprayOptionProperties::ParticleDistribution_Uniform ? 0 : 1;
        m_options->angularDistCombo->setCurrentIndex(index);
        slot_angularDistCombo_currentIndexChanged(index);
    }
    m_options->curveAngularDistWidget->setCurve(op.angularDistributionCurve());
    m_options->curveAngularDistSpin->setValue(op.angularDistributionCurveRepeat());
    {
        int index;
        if (op.radialDistributionType() == KisSprayOptionProperties::ParticleDistribution_Uniform) {
            index = 0;
        } else if (op.radialDistributionType() == KisSprayOptionProperties::ParticleDistribution_Gaussian) {
            index = 1;
        } else if (op.radialDistributionType() == KisSprayOptionProperties::ParticleDistribution_ClusterBased) {
            index = 2;
        } else {
            index = 3;
        }
        m_options->radialDistCombo->setCurrentIndex(index);
        slot_radialDistCombo_currentIndexChanged(index);
    }
    m_options->stdDeviationRadialDistSpin->setValue(op.radialDistributionStdDeviation());
    m_options->clusterRadialDistSpin->setValue(op.radialDistributionClusteringAmount());
    m_options->curveRadialDistWidget->setCurve(op.radialDistributionCurve());
    m_options->curveRadialDistSpin->setValue(op.radialDistributionCurveRepeat());
    m_options->centerBiasedPolarDistanceBox->setChecked(op.radialDistributionCenterBiased());
}


void KisSprayOpOption::setDiameter(int diameter) const
{
    m_options->diameterSpinBox->setValue(diameter);
}

int KisSprayOpOption::diameter() const
{
    return m_options->diameterSpinBox->value();
}

qreal KisSprayOpOption::brushAspect() const
{
    return m_options->aspectSPBox->value();
}

void KisSprayOpOption::slot_angularDistCombo_currentIndexChanged(int index)
{
    if (index == 0 && m_options->layoutAngularDist->count() == 3) {
        m_options->layoutAngularDist->takeAt(1);
        m_options->curveAngularDistContainer->setVisible(false);
    } else if (index == 1 && m_options->layoutAngularDist->count() == 2) {
        m_options->layoutAngularDist->insertWidget(1, m_options->curveAngularDistContainer, 0);
        m_options->curveAngularDistContainer->setVisible(true);
    }
}

void KisSprayOpOption::slot_radialDistCombo_currentIndexChanged(int index)
{
    while (m_options->layoutRadialDist->count() > 2) {
        m_options->layoutRadialDist->takeAt(1)->widget()->setVisible(false);
    }
    if (index == 0) {
        m_options->layoutRadialDist->insertWidget(1, m_options->centerBiasedPolarDistanceBox, 0);
        m_options->centerBiasedPolarDistanceBox->setVisible(true);
    } else if (index == 1) {
        m_options->layoutRadialDist->insertWidget(1, m_options->centerBiasedPolarDistanceBox, 0);
        m_options->layoutRadialDist->insertWidget(1, m_options->stdDeviationRadialDistSpin, 0);
        m_options->centerBiasedPolarDistanceBox->setVisible(true);
        m_options->stdDeviationRadialDistSpin->setVisible(true);
    } else if (index == 2) {
        m_options->layoutRadialDist->insertWidget(1, m_options->clusterRadialDistSpin, 0);
        m_options->clusterRadialDistSpin->setVisible(true);
    } else if (index == 3) {
        m_options->layoutRadialDist->insertWidget(1, m_options->curveRadialDistContainer, 0);
        m_options->curveRadialDistContainer->setVisible(true);
    }
}

quint16 KisSprayOptionProperties::diameter() const
{
    return m_diameter;
}

int KisSprayOptionProperties::radius() const {
    return m_diameter / 2;
}

qreal KisSprayOptionProperties::aspect() const
{
    return m_aspect;
}

qreal KisSprayOptionProperties::brushRotation() const
{
    return m_brushRotation;
}

qreal KisSprayOptionProperties::scale() const
{
    return m_scale;
}

qreal KisSprayOptionProperties::spacing() const
{
    return m_spacing;
}

bool KisSprayOptionProperties::jitterMovement() const
{
    return m_jitterMovement;
}

qreal KisSprayOptionProperties::jitterAmount() const
{
    return m_jitterAmount;
}

quint16 KisSprayOptionProperties::particleCount() const
{
    return m_particleCount;
}

qreal KisSprayOptionProperties::coverage() const
{
    return m_coverage;
}

bool KisSprayOptionProperties::useDensity() const
{
    return m_useDensity;
}

KisSprayOptionProperties::ParticleDistribution KisSprayOptionProperties::angularDistributionType() const
{
    return m_angularDistributionType;
}

KisCubicCurve KisSprayOptionProperties::angularDistributionCurve() const
{
    return m_angularDistributionCurve;
}

int KisSprayOptionProperties::angularDistributionCurveRepeat() const
{
    return m_angularDistributionCurveRepeat;
}

KisSprayOptionProperties::ParticleDistribution KisSprayOptionProperties::radialDistributionType() const
{
    return m_radialDistributionType;
}

qreal KisSprayOptionProperties::radialDistributionStdDeviation() const
{
    return m_radialDistributionStdDeviation;
}

qreal KisSprayOptionProperties::radialDistributionClusteringAmount() const
{
    return m_radialDistributionClusteringAmount;
}

KisCubicCurve KisSprayOptionProperties::radialDistributionCurve() const
{
    return m_radialDistributionCurve;
}

int KisSprayOptionProperties::radialDistributionCurveRepeat() const
{
    return m_radialDistributionCurveRepeat;
}

bool KisSprayOptionProperties::radialDistributionCenterBiased() const
{
    return m_radialDistributionCenterBiased;
}

void KisSprayOptionProperties::setDiameter(quint16 newDiameter)
{
    m_diameter = newDiameter;
}

void KisSprayOptionProperties::setAspect(qreal newAspect)
{
    m_aspect = newAspect;
}

void KisSprayOptionProperties::setBrushRotation(qreal newBrushRotation)
{
    m_brushRotation = newBrushRotation;
}

void KisSprayOptionProperties::setScale(qreal newScale)
{
    m_scale = newScale;
}

void KisSprayOptionProperties::setSpacing(qreal newSpacing)
{
    m_spacing = newSpacing;
}

void KisSprayOptionProperties::setJitterMovement(bool newJitterMovement)
{
    m_jitterMovement = newJitterMovement;
}

void KisSprayOptionProperties::setJitterAmount(qreal newJitterAmount)
{
    m_jitterAmount = newJitterAmount;
}

void KisSprayOptionProperties::setParticleCount(quint16 newParticleCount)
{
    m_particleCount = newParticleCount;
}

void KisSprayOptionProperties::setCoverage(qreal newCoverage)
{
    m_coverage = newCoverage;
}

void KisSprayOptionProperties::setUseDensity(bool newUseDensity)
{
    m_useDensity = newUseDensity;
}

void KisSprayOptionProperties::setAngularDistributionType(ParticleDistribution newAngularDistributionType)
{
    m_angularDistributionType = newAngularDistributionType;
}

void KisSprayOptionProperties::setAngularDistributionCurve(KisCubicCurve newAngularDistributionCurve)
{
    m_angularDistributionCurve = newAngularDistributionCurve;
}

void KisSprayOptionProperties::setAngularDistributionCurveRepeat(int newAngularDistributionCurveRepeat)
{
    m_angularDistributionCurveRepeat = newAngularDistributionCurveRepeat;
}

void KisSprayOptionProperties::setRadialDistributionType(ParticleDistribution newRadialDistributionType)
{
    m_radialDistributionType = newRadialDistributionType;
}

void KisSprayOptionProperties::setRadialDistributionStdDeviation(qreal newRadialDistributionStdDeviation)
{
    m_radialDistributionStdDeviation = newRadialDistributionStdDeviation;
}

void KisSprayOptionProperties::setRadialDistributionClusteringAmount(qreal newRadialDistributionClusteringAmount)
{
    m_radialDistributionClusteringAmount = newRadialDistributionClusteringAmount;
}

void KisSprayOptionProperties::setRadialDistributionCurve(KisCubicCurve newRadialDistributionCurve)
{
    m_radialDistributionCurve = newRadialDistributionCurve;
}

void KisSprayOptionProperties::setRadialDistributionCurveRepeat(int newRadialDistributionCurveRepeat)
{
    m_radialDistributionCurveRepeat = newRadialDistributionCurveRepeat;
}

void KisSprayOptionProperties::setRadialDistributionCenterBiased(bool newRadialDistributionCenterBiased)
{
    m_radialDistributionCenterBiased = newRadialDistributionCenterBiased;
}

const KisSprayUniformDistribution& KisSprayOptionProperties::uniformDistribution() const
{
    return m_uniformDistribution;
}

const KisSprayCurveBasedDistribution& KisSprayOptionProperties::angularCurveBasedDistribution() const
{
    return m_angularCurveBasedDistribution;
}

const KisSprayUniformDistributionPolarDistance& KisSprayOptionProperties::uniformDistributionPolarDistance() const
{
    return m_uniformDistributionPolarDistance;
}

const KisSprayNormalDistribution& KisSprayOptionProperties::normalDistribution() const
{
    return m_normalDistribution;
}

const KisSprayNormalDistributionPolarDistance& KisSprayOptionProperties::normalDistributionPolarDistance() const
{
    return m_normalDistributionPolarDistance;
}

const KisSprayClusterBasedDistributionPolarDistance& KisSprayOptionProperties::clusterBasedDistributionPolarDistance() const
{
    return m_clusterBasedDistributionPolarDistance;
}

const KisSprayCurveBasedDistributionPolarDistance& KisSprayOptionProperties::radialCurveBasedDistributionPolarDistance() const
{
    return m_radialCurveBasedDistributionPolarDistance;
}

void KisSprayOptionProperties::updateDistributions()
{
    if (angularDistributionType() == ParticleDistribution_CurveBased) {
        m_angularCurveBasedDistribution = KisSprayCurveBasedDistribution(angularDistributionCurve(), angularDistributionCurveRepeat());
    }
    if (radialDistributionType() == ParticleDistribution_Gaussian) {
        if (radialDistributionCenterBiased()) {
            m_normalDistribution = KisSprayNormalDistribution(0.0, radialDistributionStdDeviation());
        } else {
            m_normalDistributionPolarDistance = KisSprayNormalDistributionPolarDistance(0.0, radialDistributionStdDeviation());
        }
    } else if (radialDistributionType() == ParticleDistribution_ClusterBased) {
        m_clusterBasedDistributionPolarDistance = KisSprayClusterBasedDistributionPolarDistance(radialDistributionClusteringAmount());
    } else if (radialDistributionType() == ParticleDistribution_CurveBased) {
        m_radialCurveBasedDistributionPolarDistance = KisSprayCurveBasedDistributionPolarDistance(radialDistributionCurve(), radialDistributionCurveRepeat());
    }
}

void KisSprayOptionProperties::readOptionSettingImpl(const KisPropertiesConfiguration *settings)
{
    QVariant v;
    setDiameter(settings->getInt(SPRAY_DIAMETER));
    setAspect(settings->getDouble(SPRAY_ASPECT));
    setBrushRotation(settings->getDouble(SPRAY_ROTATION));
    setScale(settings->getDouble(SPRAY_SCALE));
    setSpacing(settings->getDouble(SPRAY_SPACING));
    setJitterMovement(settings->getBool(SPRAY_JITTER_MOVEMENT));
    setJitterAmount(settings->getDouble(SPRAY_JITTER_MOVE_AMOUNT));
    setParticleCount(settings->getDouble(SPRAY_PARTICLE_COUNT));
    setCoverage(settings->getDouble(SPRAY_COVERAGE) / 100.0);
    setUseDensity(settings->getBool(SPRAY_USE_DENSITY));
    {
        const QString angularDistributionTypeStr = settings->getString(SPRAY_ANGULAR_DISTRIBUTION_TYPE, "uniform");
        if (angularDistributionTypeStr == "curveBased") {
            setAngularDistributionType(ParticleDistribution_CurveBased);
        } else {
            setAngularDistributionType(ParticleDistribution_Uniform);
        }
        setAngularDistributionCurve(settings->getCubicCurve(SPRAY_ANGULAR_DISTRIBUTION_CURVE, KisCubicCurve(QList<QPointF>{{0.0, 1.0}, {1.0, 0.0}})));
        setAngularDistributionCurveRepeat(settings->getInt(SPRAY_ANGULAR_DISTRIBUTION_CURVE_REPEAT, 1));
    }
    {
        const QString radialDistributionTypeStr = settings->getString(SPRAY_RADIAL_DISTRIBUTION_TYPE, "");
        if (radialDistributionTypeStr == "uniform") {
            setRadialDistributionType(ParticleDistribution_Uniform);
        } else if (radialDistributionTypeStr == "gaussian") {
            setRadialDistributionType(ParticleDistribution_Gaussian);
        } else if (radialDistributionTypeStr == "clusterBased") {
            setRadialDistributionType(ParticleDistribution_ClusterBased);
        } else if (radialDistributionTypeStr == "curveBased") {
            setRadialDistributionType(ParticleDistribution_CurveBased);
        } else {
            // Old brush
            if (settings->getBool(SPRAY_GAUSS_DISTRIBUTION, false)) {
                setRadialDistributionType(ParticleDistribution_Gaussian);
            } else {
                setRadialDistributionType(ParticleDistribution_Uniform);
            }
        }
        setRadialDistributionStdDeviation(settings->getDouble(SPRAY_RADIAL_DISTRIBUTION_STD_DEVIATION, 0.5));
        setRadialDistributionClusteringAmount(settings->getDouble(SPRAY_RADIAL_DISTRIBUTION_CLUSTERING_AMOUNT, 0.0));
        setRadialDistributionCurve(settings->getCubicCurve(SPRAY_RADIAL_DISTRIBUTION_CURVE, KisCubicCurve(QList<QPointF>{{0.0, 1.0}, {1.0, 0.0}})));
        setRadialDistributionCurveRepeat(settings->getInt(SPRAY_RADIAL_DISTRIBUTION_CURVE_REPEAT, 1));
        setRadialDistributionCenterBiased(settings->getBool(SPRAY_RADIAL_DISTRIBUTION_CENTER_BIASED, true));
    }
}

void KisSprayOptionProperties::writeOptionSettingImpl(KisPropertiesConfiguration *setting) const
{
    setting->setProperty(SPRAY_DIAMETER, m_diameter);
    setting->setProperty(SPRAY_ASPECT, m_aspect);
    setting->setProperty(SPRAY_ROTATION, m_brushRotation);
    setting->setProperty(SPRAY_SCALE, m_scale);
    setting->setProperty(SPRAY_SPACING, m_spacing);
    setting->setProperty(SPRAY_JITTER_MOVEMENT, m_jitterMovement);
    setting->setProperty(SPRAY_JITTER_MOVE_AMOUNT, m_jitterAmount);
    setting->setProperty(SPRAY_PARTICLE_COUNT, m_particleCount);
    setting->setProperty(SPRAY_COVERAGE, m_coverage * 100.0);
    setting->setProperty(SPRAY_USE_DENSITY, m_useDensity);
    if (m_angularDistributionType == ParticleDistribution_CurveBased) {
        setting->setProperty(SPRAY_ANGULAR_DISTRIBUTION_TYPE, "curveBased");
    } else {
        setting->setProperty(SPRAY_ANGULAR_DISTRIBUTION_TYPE, "uniform");
    }
    if (m_radialDistributionType == ParticleDistribution_Gaussian) {
        setting->setProperty(SPRAY_GAUSS_DISTRIBUTION, true);
        setting->setProperty(SPRAY_RADIAL_DISTRIBUTION_TYPE, "gaussian");
    } else if (m_radialDistributionType == ParticleDistribution_ClusterBased) {
        setting->setProperty(SPRAY_RADIAL_DISTRIBUTION_TYPE, "clusterBased");
    } else if (m_radialDistributionType == ParticleDistribution_CurveBased) {
        setting->setProperty(SPRAY_RADIAL_DISTRIBUTION_TYPE, "curveBased");
    } else {
        setting->setProperty(SPRAY_RADIAL_DISTRIBUTION_TYPE, "uniform");
    }
    QVariant c;
    c.setValue(m_angularDistributionCurve);
    setting->setProperty(SPRAY_ANGULAR_DISTRIBUTION_CURVE, c);
    setting->setProperty(SPRAY_ANGULAR_DISTRIBUTION_CURVE_REPEAT, m_angularDistributionCurveRepeat);
    setting->setProperty(SPRAY_RADIAL_DISTRIBUTION_STD_DEVIATION, m_radialDistributionStdDeviation);
    setting->setProperty(SPRAY_RADIAL_DISTRIBUTION_CLUSTERING_AMOUNT, m_radialDistributionClusteringAmount);
    c.setValue(m_radialDistributionCurve);
    setting->setProperty(SPRAY_RADIAL_DISTRIBUTION_CURVE, c);
    setting->setProperty(SPRAY_RADIAL_DISTRIBUTION_CURVE_REPEAT, m_radialDistributionCurveRepeat);
    setting->setProperty(SPRAY_GAUSS_DISTRIBUTION, m_radialDistributionType == ParticleDistribution_Gaussian);
    setting->setProperty(SPRAY_RADIAL_DISTRIBUTION_CENTER_BIASED, m_radialDistributionCenterBiased);
}
