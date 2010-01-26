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
#include "kis_sumi_ink_option.h"
#include "kis_sumi_paintop_settings.h"

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

KisSumiInkOption::KisSumiInkOption()
        : KisPaintOpOption(i18n("Ink depletion"), false)
{
    m_checkable = false;
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
    
    setConfigurationPage(m_options);
}

KisSumiInkOption::~KisSumiInkOption()
{
    delete m_options;
}


void KisSumiInkOption::readOptionSetting(const KisPropertiesConfiguration* settings)
{
    m_options->inkAmountSpinBox->setValue(settings->getInt(SUMI_INK_AMOUNT));
    m_options->saturationCBox->setChecked(settings->getBool(SUMI_INK_USE_SATURATION));
    m_options->opacityCBox->setChecked(settings->getBool(SUMI_INK_USE_OPACITY));
    m_options->useWeightCHBox->setChecked(settings->getBool(SUMI_INK_USE_WEIGHTS));
    m_options->pressureSlider->setValue(settings->getInt(SUMI_INK_PRESSURE_WEIGHT));
    m_options->bristleLengthSlider->setValue(settings->getInt(SUMI_INK_BRISTLE_LENGTH_WEIGHT));
    m_options->bristleInkAmountSlider->setValue(settings->getInt(SUMI_INK_BRISTLE_INK_AMOUNT_WEIGHT));
    m_options->inkDepletionSlider->setValue(settings->getInt(SUMI_INK_DEPLETION_WEIGHT));
    m_options->inkCurve->setCurve(settings->getCubicCurve(SUMI_INK_DEPLETION_CURVE));
}

void KisSumiInkOption::writeOptionSetting(KisPropertiesConfiguration* settings) const
{
  /*  QList<float> c = curve();
    config->setProperty("curve_count", c.count());
    for (int i = 0; i < c.count(); ++i) {
        config->setProperty(QString("ink_curve_%1").arg(i), c[i]);
    }
 */
    settings->setProperty(SUMI_INK_AMOUNT, inkAmount());
    settings->setProperty(SUMI_INK_USE_SATURATION, useSaturation());
    settings->setProperty(SUMI_INK_USE_OPACITY, useOpacity());
    settings->setProperty(SUMI_INK_USE_WEIGHTS, useWeights());
    settings->setProperty(SUMI_INK_PRESSURE_WEIGHT, pressureWeight());
    settings->setProperty(SUMI_INK_BRISTLE_LENGTH_WEIGHT, bristleLengthWeight());
    settings->setProperty(SUMI_INK_BRISTLE_INK_AMOUNT_WEIGHT, bristleInkAmountWeight());
    settings->setProperty(SUMI_INK_DEPLETION_WEIGHT, inkDepletionWeight());
    settings->setProperty(SUMI_INK_DEPLETION_CURVE, qVariantFromValue(m_options->inkCurve->curve()));
}


int KisSumiInkOption::inkAmount() const
{
    return m_options->inkAmountSpinBox->value();
}


bool KisSumiInkOption::useOpacity() const
{
    return m_options->opacityCBox->isChecked();
}


bool KisSumiInkOption::useSaturation() const
{
    return m_options->saturationCBox->isChecked();
}


bool KisSumiInkOption::useWeights() const
{
    return m_options->useWeightCHBox->isChecked();
}


int KisSumiInkOption::pressureWeight() const
{
    return m_options->pressureSlider->value();
}


int KisSumiInkOption::bristleLengthWeight() const
{
    return m_options->bristleLengthSlider->value();
}


int KisSumiInkOption::bristleInkAmountWeight() const
{
    return m_options->bristleInkAmountSlider->value();
}


int KisSumiInkOption::inkDepletionWeight() const
{
    return m_options->inkDepletionSlider->value();
}


