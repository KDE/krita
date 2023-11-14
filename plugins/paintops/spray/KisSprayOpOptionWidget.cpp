/*
 *  SPDX-FileCopyrightText: 2022 Agata Cacko <cacko.azh@gmail.com>
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 * 
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisSprayOpOptionWidget.h"

#include <QButtonGroup>
#include <QMetaProperty>

#include <lager/constant.hpp>
#include "ui_wdgsprayoptions.h"

#include "KisSprayOpOptionModel.h"
#include <KisDoubleSpinBoxPluralHelper.h>
#include "kis_curve_widget.h"
#include <KisCurveWidgetConnectionHelper.h>

namespace {

class KisSprayOptionsWidget: public QWidget, public Ui::WdgSprayOptions
{
public:
    KisSprayOptionsWidget(QWidget *parent = 0)
        : QWidget(parent) {
        setupUi(this);
        
        diameterSpinBox->setRange(1, 1000, 0);
		diameterSpinBox->setValue(100);
		diameterSpinBox->setExponentRatio(1.5);
		diameterSpinBox->setSuffix(i18n(" px"));

		aspectSPBox->setRange(0.0, 2.0, 2);
		aspectSPBox->setSingleStep(0.01);
		aspectSPBox->setValue(1.0);

		rotationAngleSelector->setDecimals(0);
		rotationAngleSelector->setIncreasingDirection(KisAngleGauge::IncreasingDirection_Clockwise);

		scaleSpin->setRange(0.0, 10.0, 2);
		scaleSpin->setSingleStep(0.01);
		scaleSpin->setValue(1.0);

		spacingSpin->setRange(0.0, 5.0, 2);
		spacingSpin->setSingleStep(0.01);
		spacingSpin->setValue(0.5);

		jitterMovementSpin->setRange(0.0,5.0, 1);
		jitterMovementSpin->setSingleStep(0.1);
		jitterMovementSpin->setValue(1.0);

		particlesSpinBox->setRange(1.0, 1000.0, 0);
		particlesSpinBox->setValue(12);
		particlesSpinBox->setExponentRatio(3.0);

		coverageSpin->setRange(0.001, 0.02, 3);
		coverageSpin->setSingleStep(0.001);
		coverageSpin->setValue(0.003);
		KisDoubleSpinBoxPluralHelper::install(coverageSpin, [](double value) {
		    return i18nc("{n} is the number value, % is the percent sign", "{n}%", value);
		});
		coverageSpin->setVisible(false);
		
		angularDistCombo->setToolTip(i18n("Select how the particles are distributed as a function of the angle to the center of the spray area."));

		curveAngularDistWidget->setToolTip(i18n(
			"Set a custom distribution of the particles."
			"\nThe horizontal axis represents the angle from 0 to 360 degrees."
			"\nThe vertical axis represents how probable it is for a particle ending at that angle."
			"\nThe higher the curve at a given angle, the more particles will end at that angle."
		));
		curveAngularDistSpin->setPrefix(i18n("Repeat: "));
		curveAngularDistSpin->setSuffix(i18nc("Times symbol, like in 10x", "x"));
		curveAngularDistSpin->setRange(1, 10);
		curveAngularDistSpin->setToolTip(i18n(
			"Set how many times should the curve repeat from 0 degrees to 360 degrees."
		));

		radialDistCombo->setToolTip(i18n("Select how the particles are distributed as a function of the distance from the center of the spray area."));


		centerBiasedPolarDistanceBox->setToolTip(i18n("Activates the old behavior where the particles are more accumulated towards the center of the spray area."));

		stdDeviationRadialDistSpin->setPrefix(i18n("Standard deviation: "));
		stdDeviationRadialDistSpin->setRange(0.01, 1.0, 2);
		stdDeviationRadialDistSpin->setSingleStep(0.01);
		stdDeviationRadialDistSpin->setToolTip(i18n(
			"Set the standard deviation for the gaussian distribution."
			"\nLower values will make the particles concentrate towards the center of the spray area."
		));

		clusterRadialDistSpin->setPrefix(i18n("Clustering amount: "));
		clusterRadialDistSpin->setRange(-100.0, 100.0, 2);
		clusterRadialDistSpin->setSoftRange(-10.0, 10.0);
		clusterRadialDistSpin->setToolTip(i18n(
			"Set how the particles should spread in the spray area."
			"\nPositive values will make the particles concentrate towards the center of the spray area."
			"\nNegative values will make the particles concentrate towards the border of the spray area."
			"\nValues near 0 will make the particles spread more uniformly."
		));

		curveRadialDistWidget->setToolTip(i18n(
			"Set a custom distribution of the particles."
			"\nThe horizontal axis represents the distance from the center to the border of the spray area."
			"\nThe vertical axis represents how probable it is for a particle ending at that distance."
			"\nThe higher the curve at a given distance, the more particles will end at that distance."
		));
		curveRadialDistSpin->setPrefix(i18n("Repeat: "));
		curveRadialDistSpin->setSuffix(i18nc("Times symbol, like in 10x", "x"));
		curveRadialDistSpin->setRange(1, 10);
		curveRadialDistSpin->setToolTip(i18n(
			"Set how many times should the curve repeat from the center to the border of the spray area."
		));

		layoutAngularDist->takeAt(1);
		
		curveAngularDistContainer->setVisible(false);
		
		layoutRadialDist->takeAt(1);
		layoutRadialDist->takeAt(1);
		layoutRadialDist->takeAt(1);
		
		stdDeviationRadialDistSpin->setVisible(false);
		clusterRadialDistSpin->setVisible(false);
		curveRadialDistContainer->setVisible(false);
	}
};
}

struct KisSprayOpOptionWidget::Private
{
    Private(lager::cursor<KisSprayOpOptionData> optionData)
        : model(optionData)
    {
    }

    KisSprayOpOptionModel model;
};

namespace {
	void slotSetupAngularDistributionWidget(KisSprayOptionsWidget *widget, int index)
	{
		if (index == 0 && widget->layoutAngularDist->count() == 3) {
			widget->layoutAngularDist->takeAt(1);
			widget->curveAngularDistContainer->setVisible(false);
		} else if (index == 1 && widget->layoutAngularDist->count() == 2) {
			widget->layoutAngularDist->insertWidget(1, widget->curveAngularDistContainer, 0);
			widget->curveAngularDistContainer->setVisible(true);
		}
	}
	
	void slotSetupRadialDistributionWidget(KisSprayOptionsWidget *widget, int index)
	{
		while (widget->layoutRadialDist->count() > 2) {
			widget->layoutRadialDist->takeAt(1)->widget()->setVisible(false);
		}
		if (index == 0) {
			widget->layoutRadialDist->insertWidget(1, widget->centerBiasedPolarDistanceBox, 0);
			widget->centerBiasedPolarDistanceBox->setVisible(true);
		} else if (index == 1) {
			widget->layoutRadialDist->insertWidget(1, widget->centerBiasedPolarDistanceBox, 0);
			widget->layoutRadialDist->insertWidget(1, widget->stdDeviationRadialDistSpin, 0);
			widget->centerBiasedPolarDistanceBox->setVisible(true);
			widget->stdDeviationRadialDistSpin->setVisible(true);
		} else if (index == 2) {
			widget->layoutRadialDist->insertWidget(1, widget->clusterRadialDistSpin, 0);
			widget->clusterRadialDistSpin->setVisible(true);
		} else if (index == 3) {
			widget->layoutRadialDist->insertWidget(1, widget->curveRadialDistContainer, 0);
			widget->curveRadialDistContainer->setVisible(true);
		}
	}
}


KisSprayOpOptionWidget::KisSprayOpOptionWidget(lager::cursor<KisSprayOpOptionData> optionData)
    : KisPaintOpOption(i18n("Spray Area"), KisPaintOpOption::GENERAL, true)
    , m_d(new Private(optionData))
{
	
	KisSprayOptionsWidget *widget = new KisSprayOptionsWidget();
	setObjectName("KisSprayOpOption");
	
	m_checkable = false;
	
    using namespace KisWidgetConnectionUtils;
    
    connectControl(widget->diameterSpinBox, &m_d->model, "diameter");
    connectControl(widget->aspectSPBox, &m_d->model, "aspect");
    connectControl(widget->rotationAngleSelector, &m_d->model, "brushRotation");
    connectControl(widget->scaleSpin, &m_d->model, "scale");
    connectControl(widget->spacingSpin, &m_d->model, "spacing");
    connectControl(widget->jitterMoveBox, &m_d->model, "jitterMovement");
    connectControl(widget->jitterMovementSpin, &m_d->model, "jitterAmount");
    
    connectControl(widget->densityRadioButton, &m_d->model, "useDensity");
    
    
    connectControl(widget->particlesSpinBox, &m_d->model, "particleCount");
    connectControl(widget->coverageSpin, &m_d->model, "coverage");
    
    
    connectControl(widget->angularDistCombo, &m_d->model, "angularDistributionType");
    
    connectControl(widget->curveAngularDistWidget, &m_d->model, "angularDistributionCurve");
    connectControl(widget->curveAngularDistSpin, &m_d->model, "angularDistributionCurveRepeat");
    
    // enable widgets for the angular distribution
    slotSetupAngularDistributionWidget(widget, m_d->model.angularDistributionType() == KisSprayOpOptionData::ParticleDistribution_Uniform ? 0 : 1);
    connect(&m_d->model, &KisSprayOpOptionModel::angularDistributionTypeChanged, widget, [this, widget] (int index) {
		slotSetupAngularDistributionWidget(widget, index);
	});
    
    
    connectControl(widget->radialDistCombo, &m_d->model, "radialDistributionType");
    connectControl(widget->stdDeviationRadialDistSpin, &m_d->model, "radialDistributionStdDeviation");
    connectControl(widget->clusterRadialDistSpin, &m_d->model, "radialDistributionClusteringAmount");
    
    connectControl(widget->curveRadialDistWidget, &m_d->model, "radialDistributionCurve");
    
    // enable widgets for radial distribution
    slotSetupRadialDistributionWidget(widget, (int)m_d->model.radialDistributionType());
    connect(&m_d->model, &KisSprayOpOptionModel::radialDistributionTypeChanged, widget, [this, widget] (int index) {
		slotSetupRadialDistributionWidget(widget, index);
	});
	
    
    connectControl(widget->curveRadialDistSpin, &m_d->model, "radialDistributionCurveRepeat");
    connectControl(widget->centerBiasedPolarDistanceBox, &m_d->model, "radialDistributionCenterBiased");
    
    widget->jitterMovementSpin->setEnabled(m_d->model.jitterMovement());
    connect(&m_d->model, &KisSprayOpOptionModel::jitterMovementChanged, widget->jitterMovementSpin, &KisDoubleSliderSpinBox::setEnabled);
    
    connectWidgetVisibleToProperty(widget->particlesSpinBox, &m_d->model, "isNumParticlesVisible");
    connectWidgetVisibleToProperty(widget->coverageSpin, &m_d->model, "useDensity");
    
    m_d->model.optionData.bind(std::bind(&KisSprayOpOptionWidget::emitSettingChanged, this));
    
    setConfigurationPage(widget);
}

KisSprayOpOptionWidget::~KisSprayOpOptionWidget()
{
}

void KisSprayOpOptionWidget::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    KisSprayOpOptionData data = *m_d->model.optionData;
    data.write(setting.data());
}

void KisSprayOpOptionWidget::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisSprayOpOptionData data = *m_d->model.optionData;
    data.read(setting.data());
    m_d->model.optionData.set(data);
}

lager::cursor<qreal> KisSprayOpOptionWidget::scale() const
{
	return m_d->model.LAGER_QT(scale);
}

lager::cursor<int> KisSprayOpOptionWidget::diameter() const
{
	return m_d->model.LAGER_QT(diameter);
}
