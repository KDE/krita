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

#include "kis_grid_color_option.h"
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

KisGridColorOption::KisGridColorOption()
        : KisPaintOpOption(i18n("Color options"), false)
{
    m_checkable = false;
    m_options = new KisColorOptionsWidget();
    
    connect(m_options->randomOpacityCHBox, SIGNAL(toggled(bool)),SIGNAL(sigSettingChanged()));
    connect(m_options->randomHSVCHBox, SIGNAL(toggled(bool)),SIGNAL(sigSettingChanged()));
    connect(m_options->hueSlider,SIGNAL(valueChanged(int)),SIGNAL(sigSettingChanged()));
    connect(m_options->saturationSlider,SIGNAL(valueChanged(int)),SIGNAL(sigSettingChanged()));
    connect(m_options->valueSlider,SIGNAL(valueChanged(int)),SIGNAL(sigSettingChanged()));
    
    setConfigurationPage(m_options);
}

KisGridColorOption::~KisGridColorOption()
{
    // delete m_options; 
}

// TODO
void KisGridColorOption::writeOptionSetting(KisPropertiesConfiguration* setting) const
{
//     setting->setProperty( "Grid/diameter", diameter() );
}

// TODO
void KisGridColorOption::readOptionSetting(const KisPropertiesConfiguration* setting)
{
/*    m_options->diameterSpinBox->setValue( setting->getInt("Grid/diameter") );
    m_options->coverageSpin->setValue( setting->getDouble("Grid/coverage") );
    m_options->jitterSizeBox->setChecked( setting->getBool("Grid/jitterSize") );*/
}



bool KisGridColorOption::useRandomOpacity() const
{
    return m_options->randomOpacityCHBox->isChecked();
}

bool KisGridColorOption::useRandomHSV() const
{
    return m_options->randomHSVCHBox->isChecked();
}

int KisGridColorOption::hue() const
{
    return m_options->hueSlider->value();
}


int KisGridColorOption::saturation() const
{
    return m_options->saturationSlider->value();
}


int KisGridColorOption::value() const
{
    return m_options->valueSlider->value();
}


bool KisGridColorOption::sampleInputColor() const
{
    return m_options->sampleInputCHBox->isChecked();
}



bool KisGridColorOption::colorPerParticle() const
{
    return m_options->colorPerParticleCHBox->isChecked();
}


bool KisGridColorOption::fillBackground() const
{
    return m_options->fillBackgroundCHBox->isChecked();
}


bool KisGridColorOption::mixBgColor() const
{
    return m_options->mixBgColorCHBox->isChecked();
}







