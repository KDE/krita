/*
*  Copyright (c) 2008 Lukas Tvrdy <lukast.dev@gmail.com>
*  Copyright (c) 2010 José Luis Vergara <pentalis@gmail.com>
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

#include "kis_hatching_preferences.h"

#include "ui_wdghatchingpreferences.h"

class KisHatchingPreferencesWidget: public QWidget, public Ui::WdgHatchingPreferences
{
    public:
        KisHatchingPreferencesWidget(QWidget *parent = 0)
        : QWidget(parent) {
            setupUi(this);
        }
};

KisHatchingPreferences::KisHatchingPreferences()
: KisPaintOpOption(i18n("Hatching preferences"), KisPaintOpOption::brushCategory(), false)
{
    m_checkable = false;
    m_options = new KisHatchingPreferencesWidget();

    /*
    connect(m_options->trigonometryAlgebraRadioButton, SIGNAL(clicked(bool)),SIGNAL(sigSettingChanged()));
    connect(m_options->scratchOffRadioButton, SIGNAL(clicked(bool)),SIGNAL(sigSettingChanged()));
    */
    
    connect(m_options->antialiasCheckBox, SIGNAL(clicked(bool)),SIGNAL(sigSettingChanged()));
    connect(m_options->opaqueBackgroundCheckBox, SIGNAL(clicked(bool)),SIGNAL(sigSettingChanged()));
    connect(m_options->subpixelPrecisionCheckBox, SIGNAL(clicked(bool)),SIGNAL(sigSettingChanged()));
    
    setConfigurationPage(m_options);
}

KisHatchingPreferences::~KisHatchingPreferences()
{
}

void KisHatchingPreferences::writeOptionSetting(KisPropertiesConfiguration* setting) const
{
    /*
    setting->setProperty("Hatching/bool_trigonometryalgebra", m_options->trigonometryAlgebraRadioButton->isChecked() );
    setting->setProperty("Hatching/bool_scratchoff", m_options->scratchOffRadioButton->isChecked() );
    */
    
    setting->setProperty("Hatching/bool_antialias", m_options->antialiasCheckBox->isChecked() );
    setting->setProperty("Hatching/bool_opaquebackground", m_options->opaqueBackgroundCheckBox->isChecked() );
    setting->setProperty("Hatching/bool_subpixelprecision", m_options->subpixelPrecisionCheckBox->isChecked() );
}

void KisHatchingPreferences::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    /*
    m_options->trigonometryAlgebraRadioButton->setChecked( setting->getBool("Hatching/bool_trigonometryalgebra") );
    m_options->scratchOffRadioButton->setChecked( setting->getBool("Hatching/bool_scratchoff") );
    */
    
    m_options->antialiasCheckBox->setChecked( setting->getBool("Hatching/bool_antialias") );
    m_options->opaqueBackgroundCheckBox->setChecked( setting->getBool("Hatching/bool_opaquebackground") );
    m_options->subpixelPrecisionCheckBox->setChecked( setting->getBool("Hatching/bool_subpixelprecision") );
}
