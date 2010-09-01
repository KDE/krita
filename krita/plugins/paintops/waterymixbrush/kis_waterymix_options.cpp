/*
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

#include "kis_waterymix_options.h"

#include "ui_wdgwaterymixoptions.h"

class KisWateryMixOptionsWidget: public QWidget, public Ui::WdgWateryMixOptions
{
public:
    KisWateryMixOptionsWidget(QWidget *parent = 0)
    : QWidget(parent) {
        setupUi(this);
    }
    
};

KisWateryMixOptions::KisWateryMixOptions()
        : KisPaintOpOption(i18n("WateryMix options"), KisPaintOpOption::brushCategory(), false)
{
    m_checkable = false;
    m_options = new KisWateryMixOptionsWidget();
    
//    connect(m_options->separationIntervalSpinBox, SIGNAL(valueChanged(int)),SIGNAL(sigSettingChanged()));
    
    setConfigurationPage(m_options);
}

KisWateryMixOptions::~KisWateryMixOptions()
{
}

void KisWateryMixOptions::writeOptionSetting(KisPropertiesConfiguration* setting) const
{
//    setting->setProperty("WateryMix/separationintervals", m_options->separationIntervalSpinBox->value() );
}

void KisWateryMixOptions::readOptionSetting(const KisPropertiesConfiguration* setting)
{
//    m_options->separationIntervalSpinBox->setValue( setting->getInt("WateryMix/separationintervals") );
}
;