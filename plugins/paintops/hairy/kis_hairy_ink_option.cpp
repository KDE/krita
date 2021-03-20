/*
 *  SPDX-FileCopyrightText: 2008-2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "kis_hairy_ink_option.h"
#include "kis_hairy_paintop_settings.h"

#include <klocalizedstring.h>
#include "ui_wdgInkOptions.h"

class KisInkOptionsWidget: public QWidget, public Ui::WdgInkOptions
{
public:
    KisInkOptionsWidget(QWidget *parent = 0)
        : QWidget(parent) {
        setupUi(this);
    }
};

KisHairyInkOption::KisHairyInkOption()
    : KisPaintOpOption(KisPaintOpOption::COLOR, false)
{
    setObjectName("KisHairyInkOption");


    m_checkable = true;
    m_options = new KisInkOptionsWidget();

    // init values for slider
    m_options->pressureSlider->setRange(0.0, 100, 0);
    m_options->pressureSlider->setValue(50);
    m_options->pressureSlider->setSuffix(i18n("%"));

    m_options->bristleLengthSlider->setRange(0, 100, 0);
    m_options->bristleLengthSlider->setValue(50);
    m_options->bristleLengthSlider->setSuffix(i18n("%"));

    m_options->bristleInkAmountSlider->setRange(0, 100, 0);
    m_options->bristleInkAmountSlider->setValue(50);
    m_options->bristleInkAmountSlider->setSuffix(i18n("%"));

    m_options->inkDepletionSlider->setRange(0, 100, 0);
    m_options->inkDepletionSlider->setValue(50);
    m_options->inkDepletionSlider->setSuffix(i18n("%"));


    connect(m_options->inkAmountSpinBox, SIGNAL(valueChanged(int)), SLOT(emitSettingChanged()));
    connect(m_options->saturationCBox, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    connect(m_options->opacityCBox, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    connect(m_options->useWeightCHBox, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    connect(m_options->pressureSlider, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->bristleLengthSlider, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->bristleInkAmountSlider, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->inkDepletionSlider, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->inkCurve, SIGNAL(modified()), SLOT(emitSettingChanged()));
    connect(m_options->soakInkCBox, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));

    setConfigurationPage(m_options);
}

KisHairyInkOption::~KisHairyInkOption()
{
    delete m_options;
}


void KisHairyInkOption::readOptionSetting(const KisPropertiesConfigurationSP settings)
{
    setChecked(settings->getBool(HAIRY_INK_DEPLETION_ENABLED));
    m_options->inkAmountSpinBox->setValue(settings->getInt(HAIRY_INK_AMOUNT));
    m_options->saturationCBox->setChecked(settings->getBool(HAIRY_INK_USE_SATURATION));
    m_options->opacityCBox->setChecked(settings->getBool(HAIRY_INK_USE_OPACITY));
    m_options->useWeightCHBox->setChecked(settings->getBool(HAIRY_INK_USE_WEIGHTS));
    m_options->pressureSlider->setValue(settings->getInt(HAIRY_INK_PRESSURE_WEIGHT));
    m_options->bristleLengthSlider->setValue(settings->getInt(HAIRY_INK_BRISTLE_LENGTH_WEIGHT));
    m_options->bristleInkAmountSlider->setValue(settings->getInt(HAIRY_INK_BRISTLE_INK_AMOUNT_WEIGHT));
    m_options->inkDepletionSlider->setValue(settings->getInt(HAIRY_INK_DEPLETION_WEIGHT));
    m_options->inkCurve->setCurve(settings->getCubicCurve(HAIRY_INK_DEPLETION_CURVE));
    m_options->soakInkCBox->setChecked(settings->getBool(HAIRY_INK_SOAK));
}

void KisHairyInkOption::writeOptionSetting(KisPropertiesConfigurationSP settings) const
{
    settings->setProperty(HAIRY_INK_DEPLETION_ENABLED, isChecked());
    settings->setProperty(HAIRY_INK_AMOUNT, inkAmount());
    settings->setProperty(HAIRY_INK_USE_SATURATION, useSaturation());
    settings->setProperty(HAIRY_INK_USE_OPACITY, useOpacity());
    settings->setProperty(HAIRY_INK_USE_WEIGHTS, useWeights());
    settings->setProperty(HAIRY_INK_PRESSURE_WEIGHT, pressureWeight());
    settings->setProperty(HAIRY_INK_BRISTLE_LENGTH_WEIGHT, bristleLengthWeight());
    settings->setProperty(HAIRY_INK_BRISTLE_INK_AMOUNT_WEIGHT, bristleInkAmountWeight());
    settings->setProperty(HAIRY_INK_DEPLETION_WEIGHT, inkDepletionWeight());
    settings->setProperty(HAIRY_INK_DEPLETION_CURVE, QVariant::fromValue(m_options->inkCurve->curve()));
    settings->setProperty(HAIRY_INK_SOAK, m_options->soakInkCBox->isChecked());
}


int KisHairyInkOption::inkAmount() const
{
    return m_options->inkAmountSpinBox->value();
}


bool KisHairyInkOption::useOpacity() const
{
    return m_options->opacityCBox->isChecked();
}


bool KisHairyInkOption::useSaturation() const
{
    return m_options->saturationCBox->isChecked();
}


bool KisHairyInkOption::useWeights() const
{
    return m_options->useWeightCHBox->isChecked();
}


int KisHairyInkOption::pressureWeight() const
{
    return m_options->pressureSlider->value();
}


int KisHairyInkOption::bristleLengthWeight() const
{
    return m_options->bristleLengthSlider->value();
}


int KisHairyInkOption::bristleInkAmountWeight() const
{
    return m_options->bristleInkAmountSlider->value();
}


int KisHairyInkOption::inkDepletionWeight() const
{
    return m_options->inkDepletionSlider->value();
}


