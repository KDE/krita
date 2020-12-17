/*
*  SPDX-FileCopyrightText: 2008 Lukas Tvrdy <lukast.dev@gmail.com>
*  SPDX-FileCopyrightText: 2010 José Luis Vergara <pentalis@gmail.com>
*
*  SPDX-License-Identifier: GPL-2.0-or-later
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
    : KisPaintOpOption(KisPaintOpOption::GENERAL, false)
{
    setObjectName("KisHatchingPreferences");

    m_checkable = false;
    m_options = new KisHatchingPreferencesWidget();

    /*
    connect(m_options->trigonometryAlgebraRadioButton, SIGNAL(clicked(bool)),SLOT(emitSettingChanged()));
    connect(m_options->scratchOffRadioButton, SIGNAL(clicked(bool)),SLOT(emitSettingChanged()));
    */

    connect(m_options->antialiasCheckBox, SIGNAL(clicked(bool)), SLOT(emitSettingChanged()));
    connect(m_options->opaqueBackgroundCheckBox, SIGNAL(clicked(bool)), SLOT(emitSettingChanged()));
    connect(m_options->subpixelPrecisionCheckBox, SIGNAL(clicked(bool)), SLOT(emitSettingChanged()));

    setConfigurationPage(m_options);
}

KisHatchingPreferences::~KisHatchingPreferences()
{
}

void KisHatchingPreferences::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    /*
    setting->setProperty("Hatching/bool_trigonometryalgebra", m_options->trigonometryAlgebraRadioButton->isChecked() );
    setting->setProperty("Hatching/bool_scratchoff", m_options->scratchOffRadioButton->isChecked() );
    */

    setting->setProperty("Hatching/bool_antialias", m_options->antialiasCheckBox->isChecked());
    setting->setProperty("Hatching/bool_opaquebackground", m_options->opaqueBackgroundCheckBox->isChecked());
    setting->setProperty("Hatching/bool_subpixelprecision", m_options->subpixelPrecisionCheckBox->isChecked());
}

void KisHatchingPreferences::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    /*
    m_options->trigonometryAlgebraRadioButton->setChecked( setting->getBool("Hatching/bool_trigonometryalgebra") );
    m_options->scratchOffRadioButton->setChecked( setting->getBool("Hatching/bool_scratchoff") );
    */

    m_options->antialiasCheckBox->setChecked(setting->getBool("Hatching/bool_antialias"));
    m_options->opaqueBackgroundCheckBox->setChecked(setting->getBool("Hatching/bool_opaquebackground"));
    m_options->subpixelPrecisionCheckBox->setChecked(setting->getBool("Hatching/bool_subpixelprecision"));
}
