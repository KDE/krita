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
#include "kis_experimentop_option.h"
#include <klocale.h>

#include "ui_wdgexperimentoptions.h"

class KisExperimentOpOptionsWidget: public QWidget, public Ui::WdgExperimentOptions
{
public:
    KisExperimentOpOptionsWidget(QWidget *parent = 0)
            : QWidget(parent) {
        setupUi(this);
        // this allows to setup the component still in designer and it is needed for showing slider
        spacingSpin->setRange(spacingSpin->minimum(), spacingSpin->maximum(), 0.25, spacingSpin->showSlider());
        scaleSpin->setRange(scaleSpin->minimum(), scaleSpin->maximum(), 0.25, spacingSpin->showSlider());
        
    }
};

KisExperimentOpOption::KisExperimentOpOption()
        : KisPaintOpOption(i18n("Brush size"), false)
{
    m_checkable = false;
    m_options = new KisExperimentOpOptionsWidget();
    connect(m_options->startSPBox,SIGNAL(valueChanged(double)),SIGNAL(sigSettingChanged()));
    connect(m_options->endSPBox,SIGNAL(valueChanged(double)),SIGNAL(sigSettingChanged()));
    connect(m_options->spacingSpin,SIGNAL(valueChanged(double)),SIGNAL(sigSettingChanged()));
    
    connect(m_options->jitterMoveBox, SIGNAL(toggled(bool)), m_options->jitterMovementSpin, SLOT(setEnabled(bool)));
    setConfigurationPage(m_options);
}

KisExperimentOpOption::~KisExperimentOpOption()
{
    delete m_options;
}



int KisExperimentOpOption::endSize() const
{
    return m_options->startSPBox->value();
}


int KisExperimentOpOption::startSize() const
{
    return m_options->endSPBox->value();
}


bool KisExperimentOpOption::jitterMovement() const
{
    return m_options->jitterMoveBox->isChecked();
}

qreal KisExperimentOpOption::jitterMoveAmount() const
{
    return m_options->jitterMovementSpin->value();
}

qreal KisExperimentOpOption::spacing() const
{
    return m_options->spacingSpin->value();
}

qreal KisExperimentOpOption::scale() const
{
    return m_options->scaleSpin->value();
}

void KisExperimentOpOption::writeOptionSetting(KisPropertiesConfiguration* setting) const
{
    setting->setProperty(EXPERIMENT_START_SIZE, startSize());
    setting->setProperty(EXPERIMENT_END_SIZE, endSize());
    setting->setProperty(EXPERIMENT_SPACING, spacing());
}

void KisExperimentOpOption::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    m_options->startSPBox->setValue(setting->getDouble(EXPERIMENT_START_SIZE));
    m_options->endSPBox->setValue(setting->getDouble(EXPERIMENT_END_SIZE));
    m_options->spacingSpin->setValue(setting->getDouble(EXPERIMENT_SPACING));
}


void KisExperimentOpOption::setDiameter(int diameter) const
{
    m_options->startSPBox->setValue(diameter);
}

