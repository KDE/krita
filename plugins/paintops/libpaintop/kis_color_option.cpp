/*
 * SPDX-FileCopyrightText: 2009, 2010 Lukáš Tvrdý (lukast.dev@gmail.com)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_color_option.h"
#include <klocalizedstring.h>

#include "ui_wdgcoloroptions.h"

class KisColorOptionsWidget: public QWidget, public Ui::WdgColorOptions
{
public:
    KisColorOptionsWidget(QWidget *parent = 0)
        : QWidget(parent) {
        setupUi(this);

        hueSlider->setRange(-180, 180);
        hueSlider->setValue(0);

        saturationSlider->setRange(-100, 100);
        saturationSlider->setValue(0);

        valueSlider->setRange(-100, 100);
        valueSlider->setValue(0);

    }
};

KisColorOption::KisColorOption()
    : KisPaintOpOption(i18n("Color options"), KisPaintOpOption::COLOR, false)
{
    m_checkable = false;
    m_options = new KisColorOptionsWidget();

    setObjectName("KisColorOption");

    // ui
    connect(m_options->randomHSVCHBox, SIGNAL(toggled(bool)), SLOT(setHSVEnabled(bool)));
    // settings
    connect(m_options->randomOpacityCHBox, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    connect(m_options->randomHSVCHBox, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    connect(m_options->hueSlider, SIGNAL(valueChanged(int)), SLOT(emitSettingChanged()));
    connect(m_options->saturationSlider, SIGNAL(valueChanged(int)), SLOT(emitSettingChanged()));
    connect(m_options->valueSlider, SIGNAL(valueChanged(int)), SLOT(emitSettingChanged()));
    connect(m_options->sampleInputCHBox, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    connect(m_options->colorPerParticleCHBox, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    connect(m_options->fillBackgroundCHBox, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    connect(m_options->mixBgColorCHBox, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));

    setConfigurationPage(m_options);
}

KisColorOption::~KisColorOption()
{
    // delete m_options;
}

void KisColorOption::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    setting->setProperty(COLOROP_HUE, hue());
    setting->setProperty(COLOROP_SATURATION, saturation());
    setting->setProperty(COLOROP_VALUE, value());

    setting->setProperty(COLOROP_USE_RANDOM_HSV, useRandomHSV());
    setting->setProperty(COLOROP_USE_RANDOM_OPACITY, useRandomOpacity());
    setting->setProperty(COLOROP_SAMPLE_COLOR, sampleInputColor());

    setting->setProperty(COLOROP_FILL_BG, fillBackground());
    setting->setProperty(COLOROP_COLOR_PER_PARTICLE, colorPerParticle());
    setting->setProperty(COLOROP_MIX_BG_COLOR, mixBgColor());
}

void KisColorOption::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    m_options->hueSlider->setValue(setting->getInt(COLOROP_HUE, 0));
    m_options->saturationSlider->setValue(setting->getInt(COLOROP_SATURATION, 0));
    m_options->valueSlider->setValue(setting->getInt(COLOROP_VALUE, 0));
    m_options->randomOpacityCHBox->setChecked(setting->getBool(COLOROP_USE_RANDOM_OPACITY));
    m_options->randomHSVCHBox->setChecked(setting->getBool(COLOROP_USE_RANDOM_HSV, false));
    setHSVEnabled(m_options->randomHSVCHBox->isChecked());
    m_options->sampleInputCHBox->setChecked(setting->getBool(COLOROP_SAMPLE_COLOR, false));
    m_options->fillBackgroundCHBox->setChecked(setting->getBool(COLOROP_FILL_BG, false));
    m_options->colorPerParticleCHBox->setChecked(setting->getBool(COLOROP_COLOR_PER_PARTICLE, false));
    m_options->mixBgColorCHBox->setChecked(setting->getBool(COLOROP_MIX_BG_COLOR, false));
}

void KisColorOption::setHSVEnabled(bool enabled)
{
    m_options->hueSlider->setEnabled(enabled);
    m_options->saturationSlider->setEnabled(enabled);
    m_options->valueSlider->setEnabled(enabled);
}

bool KisColorOption::useRandomOpacity() const
{
    return m_options->randomOpacityCHBox->isChecked();
}

bool KisColorOption::useRandomHSV() const
{
    return m_options->randomHSVCHBox->isChecked();
}

int KisColorOption::hue() const
{
    return m_options->hueSlider->value();
}


int KisColorOption::saturation() const
{
    return m_options->saturationSlider->value();
}


int KisColorOption::value() const
{
    return m_options->valueSlider->value();
}


bool KisColorOption::sampleInputColor() const
{
    return m_options->sampleInputCHBox->isChecked();
}



bool KisColorOption::colorPerParticle() const
{
    return m_options->colorPerParticleCHBox->isChecked();
}


bool KisColorOption::fillBackground() const
{
    return m_options->fillBackgroundCHBox->isChecked();
}


bool KisColorOption::mixBgColor() const
{
    return m_options->mixBgColorCHBox->isChecked();
}

void KisColorProperties::fillProperties(const KisPropertiesConfigurationSP setting)
{
    hue = setting->getInt(COLOROP_HUE, 0);
    saturation = setting->getInt(COLOROP_SATURATION, 0);
    value = setting->getInt(COLOROP_VALUE, 0);
    useRandomOpacity = setting->getBool(COLOROP_USE_RANDOM_OPACITY, false);
    useRandomHSV = setting->getBool(COLOROP_USE_RANDOM_HSV, false);
    sampleInputColor = setting->getBool(COLOROP_SAMPLE_COLOR, false);
    fillBackground = setting->getBool(COLOROP_FILL_BG, false);
    colorPerParticle = setting->getBool(COLOROP_COLOR_PER_PARTICLE, false);
    mixBgColor = setting->getBool(COLOROP_MIX_BG_COLOR, false);
}
