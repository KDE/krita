/*
 *  Copyright (c) 2008,2009,2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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
    connect(m_options->aspectSPBox, SIGNAL(valueChanged(double)),SIGNAL(sigSettingChanged()));
    connect(m_options->rotationSPBox, SIGNAL(valueChanged(double)),SIGNAL(sigSettingChanged()));
    connect(m_options->jitterMoveBox, SIGNAL(toggled(bool)),SIGNAL(sigSettingChanged()));
    
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
    setting->setProperty(SPRAY_DIAMETER, diameter());
    setting->setProperty(SPRAY_ASPECT, aspect());
    setting->setProperty(SPRAY_COVERAGE, coverage());
    setting->setProperty(SPRAY_SCALE, scale());
    setting->setProperty(SPRAY_ROTATION, rotation());
    setting->setProperty(SPRAY_PARTICLE_COUNT, particleCount());
    setting->setProperty(SPRAY_JITTER_MOVE_AMOUNT, jitterMoveAmount());
    setting->setProperty(SPRAY_JITTER_MOVEMENT, jitterMovement());
    setting->setProperty(SPRAY_SPACING, spacing());
    setting->setProperty(SPRAY_GAUSS_DISTRIBUTION, gaussian());
    setting->setProperty(SPRAY_USE_DENSITY, useDensity());
}

void KisSprayOpOption::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    m_options->diameterSpinBox->setValue(setting->getInt(SPRAY_DIAMETER));
    m_options->aspectSPBox->setValue(setting->getDouble(SPRAY_ASPECT));
    m_options->coverageSpin->setValue(setting->getDouble(SPRAY_COVERAGE));
    m_options->scaleSpin->setValue(setting->getDouble(SPRAY_SCALE));
    m_options->rotationSPBox->setValue(setting->getDouble(SPRAY_ROTATION));
    m_options->particlesSpinBox->setValue(setting->getDouble(SPRAY_PARTICLE_COUNT));
    m_options->jitterMovementSpin->setValue(setting->getDouble(SPRAY_JITTER_MOVE_AMOUNT));
    m_options->jitterMoveBox->setChecked(setting->getBool(SPRAY_JITTER_MOVEMENT));
    m_options->spacingSpin->setValue(setting->getDouble(SPRAY_SPACING));
    m_options->gaussianBox->setChecked(setting->getBool(SPRAY_GAUSS_DISTRIBUTION));
    //TODO: come on, do this nicer! e.g. button group or something
    bool useDensity = setting->getBool(SPRAY_USE_DENSITY);
    m_options->densityRadioButton->setChecked(useDensity);
    m_options->countRadioButton->setChecked(!useDensity);
}


void KisSprayOpOption::setDiamter(int diameter) const
{
    m_options->diameterSpinBox->setValue(diameter);
}

