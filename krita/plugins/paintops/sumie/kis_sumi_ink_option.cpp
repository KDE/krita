/*
 *  Copyright (c) 2008,2009 Lukáš Tvrdý <lukast.dev@gmail.com>
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
    connect(m_options->weightSaturationCBox, SIGNAL(toggled(bool)), SIGNAL(sigSettingChanged()));
    connect(m_options->pressureSlider, SIGNAL(valueChanged(int)), SIGNAL(sigSettingChanged()));
    connect(m_options->bristleLengthSlider, SIGNAL(valueChanged(int)), SIGNAL(sigSettingChanged()));
    connect(m_options->bristleInkAmountSlider, SIGNAL(valueChanged(int)), SIGNAL(sigSettingChanged()));
    connect(m_options->inkDepletionSlider, SIGNAL(valueChanged(int)), SIGNAL(sigSettingChanged()));

    setConfigurationPage(m_options);
}

KisSumiInkOption::~KisSumiInkOption()
{
    // delete m_options;
}


void KisSumiInkOption::readOptionSetting(const KisPropertiesConfiguration* config)
{
    // XXX: Use a regular curve option here!
    QList<float> c;
    int count = config->getInt("curve_count");
    for (int i = 0; i < count; ++i) {
        c << config->getFloat(QString("ink_curve_%1").arg(i));
    }

    m_options->inkAmountSpinBox->setValue(config->getInt("ink_amount"));
    m_options->saturationCBox->setChecked(config->getBool("use_saturation"));
    m_options->opacityCBox->setChecked(config->getBool("use_opacity"));
    m_options->weightSaturationCBox->setChecked(config->getBool("use_weights"));
    m_options->pressureSlider->setValue(config->getInt("pressure_weight"));
    m_options->bristleLengthSlider->setValue(config->getInt("bristle_length_weight"));
    m_options->bristleInkAmountSlider->setValue(config->getInt("bristle_ink_amount_weight"));
    m_options->inkDepletionSlider->setValue(config->getInt("ink_depletion_weight"));
}

void KisSumiInkOption::writeOptionSetting(KisPropertiesConfiguration* config) const
{
    QList<float> c = curve();
    config->setProperty("curve_count", c.count());
    for (int i = 0; i < c.count(); ++i) {
        config->setProperty(QString("ink_curve_%1").arg(i), c[i]);
    }

    config->setProperty("ink_amount", inkAmount());
    config->setProperty("use_saturation", useSaturation());
    config->setProperty("use_opacity", useOpacity());
    config->setProperty("use_weights", useWeights());
    config->setProperty("pressure_weight", pressureWeight());
    config->setProperty("bristle_length_weight", bristleLengthWeight());
    config->setProperty("bristle_ink_amount_weight", bristleInkAmountWeight());
    config->setProperty("ink_depletion_weight", inkDepletionWeight());
}


int KisSumiInkOption::inkAmount() const
{
    return m_options->inkAmountSpinBox->value();
}


QList< float > KisSumiInkOption::curve() const
{
    int curveSamples = inkAmount();
    QList<float> result;
    for (int i = 0; i < curveSamples ; i++) {
        result.append((float)m_options->inkCurve->getCurveValue(i / (float)(curveSamples - 1.0f)));
    }
    return result;
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
    return m_options->weightSaturationCBox->isChecked();
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


