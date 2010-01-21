/*
 * Copyright (c) 2009,2010 Lukáš Tvrdý (lukast.dev@gmail.com)
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

#include "kis_color_option.h"
#include <klocale.h>

#include "ui_wdgcoloroptions.h"

class KisColorOptionsWidget: public QWidget, public Ui::WdgColorOptions
{
public:
    KisColorOptionsWidget(QWidget *parent = 0)
        : QWidget(parent)
    {
        setupUi(this);
    }
};

KisColorOption::KisColorOption()
        : KisPaintOpOption(i18n("Color options"), false)
{
    m_checkable = false;
    m_options = new KisColorOptionsWidget();
    
    connect(m_options->randomOpacityCHBox, SIGNAL(toggled(bool)),SIGNAL(sigSettingChanged()));
    connect(m_options->randomHSVCHBox, SIGNAL(toggled(bool)),SIGNAL(sigSettingChanged()));
    connect(m_options->hueSlider,SIGNAL(sliderReleased()),SIGNAL(sigSettingChanged()));
    connect(m_options->saturationSlider,SIGNAL(sliderReleased()),SIGNAL(sigSettingChanged()));
    connect(m_options->valueSlider,SIGNAL(sliderReleased()),SIGNAL(sigSettingChanged()));
    connect(m_options->sampleInputCHBox, SIGNAL(toggled(bool)),SIGNAL(sigSettingChanged()));
    connect(m_options->colorPerParticleCHBox, SIGNAL(toggled(bool)),SIGNAL(sigSettingChanged()));
    connect(m_options->fillBackgroundCHBox, SIGNAL(toggled(bool)),SIGNAL(sigSettingChanged()));
    connect(m_options->mixBgColorCHBox, SIGNAL(toggled(bool)),SIGNAL(sigSettingChanged()));
    
    setConfigurationPage(m_options);
}

KisColorOption::~KisColorOption()
{
    // delete m_options; 
}

void KisColorOption::writeOptionSetting(KisPropertiesConfiguration* setting) const
{
    setting->setProperty(COLOROP_HUE, hue());
    setting->setProperty(COLOROP_SATURATION, saturation());
    setting->setProperty(COLOROP_VALUE, value());
    
    setting->setProperty(COLOROP_USE_RANDOM_HSV,useRandomHSV());
    setting->setProperty(COLOROP_USE_RANDOM_OPACITY,useRandomOpacity());
    setting->setProperty(COLOROP_SAMPLE_COLOR,sampleInputColor());
    
    setting->setProperty(COLOROP_FILL_BG,fillBackground());
    setting->setProperty(COLOROP_COLOR_PER_PARTICLE, colorPerParticle());
    setting->setProperty(COLOROP_MIX_BG_COLOR,mixBgColor());
}

void KisColorOption::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    m_options->hueSlider->setValue(setting->getInt(COLOROP_HUE));
    m_options->saturationSlider->setValue(setting->getInt(COLOROP_SATURATION));
    m_options->valueSlider->setValue(setting->getInt(COLOROP_VALUE));
    m_options->randomOpacityCHBox->setChecked(setting->getBool(COLOROP_USE_RANDOM_OPACITY));
    m_options->randomHSVCHBox->setChecked(setting->getBool(COLOROP_USE_RANDOM_HSV));
    m_options->sampleInputCHBox->setChecked(setting->getBool(COLOROP_SAMPLE_COLOR));
    m_options->fillBackgroundCHBox->setChecked(setting->getBool(COLOROP_FILL_BG));
    m_options->colorPerParticleCHBox->setChecked(setting->getBool(COLOROP_COLOR_PER_PARTICLE));
    m_options->mixBgColorCHBox->setChecked(setting->getBool(COLOROP_MIX_BG_COLOR));
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

void KisColorProperties::fillProperties(const KisPropertiesConfiguration* setting)
{
    hue = setting->getInt(COLOROP_HUE);
    saturation = setting->getInt(COLOROP_SATURATION);
    value = setting->getInt(COLOROP_VALUE);
    useRandomOpacity = setting->getBool(COLOROP_USE_RANDOM_OPACITY);
    useRandomHSV = setting->getBool(COLOROP_USE_RANDOM_HSV);
    sampleInputColor = setting->getBool(COLOROP_SAMPLE_COLOR);
    fillBackground = setting->getBool(COLOROP_FILL_BG);
    colorPerParticle = setting->getBool(COLOROP_COLOR_PER_PARTICLE);
    mixBgColor = setting->getBool(COLOROP_MIX_BG_COLOR);
}
