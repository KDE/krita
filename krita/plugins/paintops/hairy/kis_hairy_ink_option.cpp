/*
 *  Copyright (c) 2008-2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "kis_hairy_ink_option.h"
#include "kis_hairy_paintop_settings.h"

#include <klocale.h>
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
        : KisPaintOpOption(i18n("Ink depletion"), KisPaintOpOption::colorCategory(), false)
{
    m_checkable = true;
    m_options = new KisInkOptionsWidget();

    connect(m_options->inkAmountSpinBox, SIGNAL(valueChanged(int)), SIGNAL(sigSettingChanged()));
    connect(m_options->saturationCBox, SIGNAL(toggled(bool)), SIGNAL(sigSettingChanged()));
    connect(m_options->opacityCBox, SIGNAL(toggled(bool)), SIGNAL(sigSettingChanged()));
    connect(m_options->useWeightCHBox, SIGNAL(toggled(bool)), SIGNAL(sigSettingChanged()));
    connect(m_options->pressureSlider, SIGNAL(valueChanged(int)), SIGNAL(sigSettingChanged()));
    connect(m_options->bristleLengthSlider, SIGNAL(valueChanged(int)), SIGNAL(sigSettingChanged()));
    connect(m_options->bristleInkAmountSlider, SIGNAL(valueChanged(int)), SIGNAL(sigSettingChanged()));
    connect(m_options->inkDepletionSlider, SIGNAL(valueChanged(int)), SIGNAL(sigSettingChanged()));
    connect(m_options->inkCurve, SIGNAL(modified()),SIGNAL(sigSettingChanged()));
    connect(m_options->soakInkCBox, SIGNAL(toggled(bool)),SIGNAL(sigSettingChanged()));
    
    setConfigurationPage(m_options);
}

KisHairyInkOption::~KisHairyInkOption()
{
    delete m_options;
}


void KisHairyInkOption::readOptionSetting(const KisPropertiesConfiguration* settings)
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

void KisHairyInkOption::writeOptionSetting(KisPropertiesConfiguration* settings) const
{
    settings->setProperty(HAIRY_INK_DEPLETION_ENABLED, isChecked() );
    settings->setProperty(HAIRY_INK_AMOUNT, inkAmount());
    settings->setProperty(HAIRY_INK_USE_SATURATION, useSaturation());
    settings->setProperty(HAIRY_INK_USE_OPACITY, useOpacity());
    settings->setProperty(HAIRY_INK_USE_WEIGHTS, useWeights());
    settings->setProperty(HAIRY_INK_PRESSURE_WEIGHT, pressureWeight());
    settings->setProperty(HAIRY_INK_BRISTLE_LENGTH_WEIGHT, bristleLengthWeight());
    settings->setProperty(HAIRY_INK_BRISTLE_INK_AMOUNT_WEIGHT, bristleInkAmountWeight());
    settings->setProperty(HAIRY_INK_DEPLETION_WEIGHT, inkDepletionWeight());
    settings->setProperty(HAIRY_INK_DEPLETION_CURVE, qVariantFromValue(m_options->inkCurve->curve()));
    settings->setProperty(HAIRY_INK_SOAK,m_options->soakInkCBox->isChecked());
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


