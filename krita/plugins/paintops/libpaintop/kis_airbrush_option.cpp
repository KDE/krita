/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
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
#include "kis_airbrush_option.h"
#include <klocale.h>

#include <QWidget>
#include <QRadioButton>

#include "ui_wdgairbrush.h"

const int MAXIMUM_RATE = 1000;

class KisAirbrushWidget: public QWidget, public Ui::WdgAirbrush
{
public:
    KisAirbrushWidget(QWidget *parent = 0)
            : QWidget(parent) {
        setupUi(this);
        
        sliderRate->setRange(0, MAXIMUM_RATE);
        sliderRate->setExponentRatio(1.8);
        sliderRate->setValue(100);
    }
};


KisAirbrushOption::KisAirbrushOption(bool enabled)
         : KisPaintOpOption(i18n("Airbrush"), KisPaintOpOption::colorCategory(), enabled)
{
    m_checkable = true;
    m_optionWidget = new KisAirbrushWidget();
    connect(m_optionWidget->sliderRate, SIGNAL(valueChanged(int)), SIGNAL(sigSettingChanged()));
    
    setConfigurationPage(m_optionWidget);
}


KisAirbrushOption::~KisAirbrushOption()
{
}


void KisAirbrushOption::writeOptionSetting(KisPropertiesConfiguration* setting) const
{
    setting->setProperty(AIRBRUSH_ENABLED, isChecked());
    setting->setProperty(AIRBRUSH_RATE, m_optionWidget->sliderRate->value());
}

void KisAirbrushOption::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    setChecked(setting->getBool(AIRBRUSH_ENABLED));
    m_optionWidget->sliderRate->setValue(setting->getInt(AIRBRUSH_RATE,100));
}

