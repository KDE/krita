/*
 *  Copyright (c) 2008-2009 Lukáš Tvrdý <lukast.dev@gmail.com>
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
#include "kis_sprayop_option.h"
#include <klocale.h>

#include "ui_wdgsprayoptions.h"

class KisSprayOpOptionsWidget: public QWidget, public Ui::WdgSprayOptions
{
public:
    KisSprayOpOptionsWidget(QWidget *parent = 0)
            : QWidget(parent) {
        setupUi(this);
        // this allows to setup the component still in designer and it is needed for showing slider
        spacingSpin->setRange(spacingSpin->minimum(), spacingSpin->maximum(), 0.25, spacingSpin->showSlider());
        scaleSpin->setRange(scaleSpin->minimum(), scaleSpin->maximum(), 0.25, spacingSpin->showSlider());
        
    }
};

KisSprayOpOption::KisSprayOpOption()
        : KisPaintOpOption(i18n("Brush size"), false)
{
    m_checkable = false;
    m_options = new KisSprayOpOptionsWidget();
    connect(m_options->diameterSpinBox, SIGNAL(valueChanged(double)), SIGNAL(sigSettingChanged()));
    connect(m_options->coverageSpin, SIGNAL(valueChanged(double)), SIGNAL(sigSettingChanged()));
    connect(m_options->jitterMovementSpin, SIGNAL(valueChanged(double)), SIGNAL(sigSettingChanged()));
    connect(m_options->spacingSpin, SIGNAL(valueChanged(double)), SIGNAL(sigSettingChanged()));
    connect(m_options->scaleSpin, SIGNAL(valueChanged(double)), SIGNAL(sigSettingChanged()));
    connect(m_options->particlesSpinBox, SIGNAL(valueChanged(double)), SIGNAL(sigSettingChanged()));
    connect(m_options->countRadioButton, SIGNAL(toggled(bool)), SIGNAL(sigSettingChanged()));
    connect(m_options->densityRadioButton, SIGNAL(toggled(bool)), SIGNAL(sigSettingChanged()));
    connect(m_options->gaussianBox, SIGNAL(toggled(bool)), SIGNAL(sigSettingChanged()));
    connect(m_options->countRadioButton, SIGNAL(toggled(bool)), m_options->particlesSpinBox, SLOT(setEnabled(bool)));
    connect(m_options->densityRadioButton, SIGNAL(toggled(bool)), m_options->coverageSpin, SLOT(setEnabled(bool)));
    connect(m_options->jitterMoveBox, SIGNAL(toggled(bool)), m_options->jitterMovementSpin, SLOT(setEnabled(bool)));
    
    setConfigurationPage(m_options);
}

KisSprayOpOption::~KisSprayOpOption()
{
    // delete m_options;
}

int KisSprayOpOption::diameter() const
{
    return m_options->diameterSpinBox->value();
}


qreal KisSprayOpOption::aspect() const
{
    return m_options->aspectSPBox->value();
}

bool KisSprayOpOption::jitterMovement() const
{
    return m_options->jitterMoveBox->isChecked();
}

qreal KisSprayOpOption::coverage() const
{
    return m_options->coverageSpin->value();
}

qreal KisSprayOpOption::jitterMoveAmount() const
{
    return m_options->jitterMovementSpin->value();
}

qreal KisSprayOpOption::spacing() const
{
    return m_options->spacingSpin->value();
}

qreal KisSprayOpOption::scale() const
{
    return m_options->scaleSpin->value();
}


qreal KisSprayOpOption::rotation() const
{
    return m_options->rotationSPBox->value();
}


int KisSprayOpOption::particleCount() const
{
    return m_options->particlesSpinBox->value();
}

bool KisSprayOpOption::useDensity() const
{
    return m_options->densityRadioButton->isChecked();
}

bool KisSprayOpOption::gaussian() const
{
    return m_options->gaussianBox->isChecked();
}

void KisSprayOpOption::writeOptionSetting(KisPropertiesConfiguration* setting) const
{
    setting->setProperty("Spray/diameter", diameter());
    setting->setProperty("Spray/coverage", coverage());
    setting->setProperty("Spray/jitterMoveAmount", jitterMoveAmount());
    setting->setProperty("Spray/spacing", spacing());
    setting->setProperty("Spray/jitterMovement", jitterMovement());
    setting->setProperty("Spray/gaussianDistribution", gaussian());
}

void KisSprayOpOption::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    m_options->diameterSpinBox->setValue(setting->getInt("Spray/diameter"));
    m_options->coverageSpin->setValue(setting->getDouble("Spray/coverage"));
    m_options->jitterMovementSpin->setValue(setting->getDouble("Spray/jitterMoveAmount"));
    m_options->spacingSpin->setValue(setting->getDouble("Spray/spacing"));
    m_options->jitterMoveBox->setChecked(setting->getBool("Spray/jitterMovement"));
    m_options->gaussianBox->setChecked(setting->getBool("Spray/gaussianDistribution"));
}


void KisSprayOpOption::setDiamter(int diameter) const
{
    m_options->diameterSpinBox->setValue(diameter);
}

