/*
 *  SPDX-FileCopyrightText: 2008, 2009, 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "kis_sprayop_option.h"
#include <klocalizedstring.h>

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
    : KisPaintOpOption(KisPaintOpOption::GENERAL, false)
{
    setObjectName("KisSprayOpOption");

    m_checkable = false;
    m_options = new KisSprayOpOptionsWidget();

    m_options->diameterSpinBox->setRange(1, 1000, 0);
    m_options->diameterSpinBox->setValue(100);
    m_options->diameterSpinBox->setExponentRatio(1.5);
    m_options->diameterSpinBox->setSuffix(i18n(" px"));

    m_options->aspectSPBox->setRange(0.0, 2.0, 2);
    m_options->aspectSPBox->setValue(1.0);

    m_options->rotationAngleSelector->setDecimals(0);
    m_options->rotationAngleSelector->setIncreasingDirection(KisAngleGauge::IncreasingDirection_Clockwise);

    m_options->scaleSpin->setRange(0.0, 10.0, 2);
    m_options->scaleSpin->setValue(1.0);

    m_options->spacingSpin->setRange(0.0, 5.0, 2);
    m_options->spacingSpin->setValue(0.5);

    m_options->coverageSpin->setRange(0.001, 0.02, 3);
    m_options->coverageSpin->setValue(0.003);
    m_options->coverageSpin->setSuffix(i18n("%"));

    m_options->particlesSpinBox->setRange(1.0, 1000.0, 0);
    m_options->particlesSpinBox->setValue(12);
    m_options->particlesSpinBox->setExponentRatio(3.0);

    m_options->jitterMovementSpin->setRange(0.0,5.0, 1);
    m_options->jitterMovementSpin->setValue(1.0);


    connect(m_options->diameterSpinBox, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->coverageSpin, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->jitterMovementSpin, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->spacingSpin, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->scaleSpin, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->particlesSpinBox, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->countRadioButton, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    connect(m_options->densityRadioButton, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    connect(m_options->gaussianBox, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    connect(m_options->aspectSPBox, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->rotationAngleSelector, SIGNAL(angleChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->jitterMoveBox, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));

    connect(m_options->countRadioButton, SIGNAL(toggled(bool)), m_options->particlesSpinBox, SLOT(setEnabled(bool)));
    connect(m_options->densityRadioButton, SIGNAL(toggled(bool)), m_options->coverageSpin, SLOT(setEnabled(bool)));
    connect(m_options->jitterMoveBox, SIGNAL(toggled(bool)), m_options->jitterMovementSpin, SLOT(setEnabled(bool)));

    setConfigurationPage(m_options);
}

KisSprayOpOption::~KisSprayOpOption()
{
    delete m_options;
}

void KisSprayOpOption::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    KisSprayOptionProperties op;

    op.diameter = m_options->diameterSpinBox->value();
    op.particleCount = m_options->particlesSpinBox->value();
    op.aspect = m_options->aspectSPBox->value();
    op.coverage = m_options->coverageSpin->value();
    op.amount = m_options->jitterMovementSpin->value();
    op.spacing = m_options->spacingSpin->value();
    op.scale = m_options->scaleSpin->value();
    op.brushRotation = m_options->rotationAngleSelector->angle();
    op.jitterMovement = m_options->jitterMoveBox->isChecked();
    op.useDensity = m_options->densityRadioButton->isChecked();
    op.gaussian = m_options->gaussianBox->isChecked();

    op.writeOptionSetting(setting);
}

void KisSprayOpOption::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisSprayOptionProperties op;
    op.readOptionSetting(setting);

    m_options->diameterSpinBox->setValue(op.diameter);
    m_options->aspectSPBox->setValue(op.aspect);
    m_options->coverageSpin->setValue(op.coverage);
    m_options->scaleSpin->setValue(op.scale);
    m_options->rotationAngleSelector->setAngle(op.brushRotation);
    m_options->particlesSpinBox->setValue(op.particleCount);
    m_options->jitterMovementSpin->setValue(op.amount);
    m_options->jitterMoveBox->setChecked(op.jitterMovement);
    m_options->spacingSpin->setValue(op.spacing);
    m_options->gaussianBox->setChecked(op.gaussian);
    //TODO: come on, do this nicer! e.g. button group or something
    bool useDensity = op.useDensity;
    m_options->densityRadioButton->setChecked(useDensity);
    m_options->countRadioButton->setChecked(!useDensity);
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
