/*
 *  Copyright (c) 2008 Lukas Tvrdy <lukast.dev@gmail.com>
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
#include "kis_hatchingop_option.h"

#include "ui_wdghatchingoptions.h"

class KisHatchingOpOptionsWidget: public QWidget, public Ui::WdgHatchingOptions
{
public:
    KisHatchingOpOptionsWidget(QWidget *parent = 0)
            : QWidget(parent) {
        setupUi(this);
    }
};

KisHatchingOpOption::KisHatchingOpOption()
        : KisPaintOpOption(i18n("Brush size"), KisPaintOpOption::brushCategory(), false)
{
    m_checkable = false;
    m_options = new KisHatchingOpOptionsWidget();
    connect(m_options->radiusSpinBox, SIGNAL(valueChanged(int)), SIGNAL(sigSettingChanged()));
    connect(m_options->inkDepletionCHBox, SIGNAL(clicked(bool)), SIGNAL(sigSettingChanged()));
    connect(m_options->opacity, SIGNAL(clicked(bool)), SIGNAL(sigSettingChanged()));
    connect(m_options->saturation, SIGNAL(clicked(bool)), SIGNAL(sigSettingChanged()));

    setConfigurationPage(m_options);
}

KisHatchingOpOption::~KisHatchingOpOption()
{
    // delete m_options;
}

int KisHatchingOpOption::radius() const
{
    return m_options->radiusSpinBox->value();
}


void KisHatchingOpOption::setRadius(int radius) const
{
    m_options->radiusSpinBox->blockSignals(true);
    m_options->radiusSpinBox->setValue( radius );
    m_options->radiusSpinBox->blockSignals(false);
}



bool KisHatchingOpOption::inkDepletion() const
{
    return m_options->inkDepletionCHBox->isChecked();
}



bool KisHatchingOpOption::opacity() const
{
    return m_options->opacity->isChecked();
}


bool KisHatchingOpOption::saturation() const
{
    return m_options->saturation->isChecked();
}


void KisHatchingOpOption::writeOptionSetting(KisPropertiesConfiguration* setting) const
{
    setting->setProperty("Hatching/radius", radius());
    setting->setProperty("Hatching/inkDepletion", inkDepletion());
    setting->setProperty("Hatching/opacity", opacity());
    setting->setProperty("Hatching/saturation", saturation());
}

void KisHatchingOpOption::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    m_options->radiusSpinBox->setValue(setting->getInt("Hatching/radius"));
    m_options->inkDepletionCHBox->setChecked(setting->getBool("Hatching/inkDepletion"));
    m_options->opacity->setChecked(setting->getBool("Hatching/opacity"));
    m_options->saturation->setChecked(setting->getBool("Hatching/saturation"));
}


