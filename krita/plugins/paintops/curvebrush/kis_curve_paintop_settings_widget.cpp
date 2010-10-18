/*
 *  Copyright (c) 2008,2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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
#include <kis_curve_paintop_settings_widget.h>

#include "kis_curve_paintop_settings.h"

KisCurvePaintOpSettingsWidget:: KisCurvePaintOpSettingsWidget(QWidget* parent)
        : KisPaintOpSettingsWidget(parent)
{
    m_options = new Ui::WdgCurveOptions();
    m_options->setupUi(this);
    
    connect(m_options->mode1Btn,SIGNAL(toggled(bool)),this, SIGNAL(sigConfigurationUpdated()));
    connect(m_options->mode2Btn,SIGNAL(toggled(bool)),this, SIGNAL(sigConfigurationUpdated()));
    connect(m_options->mode3Btn,SIGNAL(toggled(bool)),this, SIGNAL(sigConfigurationUpdated()));
    connect(m_options->minDistSPBox,SIGNAL(valueChanged(int)),this, SIGNAL(sigConfigurationUpdated()));
    connect(m_options->pulseSPBox, SIGNAL(valueChanged(int)), this, SIGNAL(sigConfigurationUpdated()));
}

KisCurvePaintOpSettingsWidget::~ KisCurvePaintOpSettingsWidget()
{
    delete m_options;
}

void  KisCurvePaintOpSettingsWidget::setConfiguration(const KisPropertiesConfiguration * config)
{
    m_options->minDistSPBox->setValue(config->getInt(CURVE_MIN_DISTANCE));
    m_options->pulseSPBox->setValue(config->getInt(CURVE_INTERVAL));

    switch (config->getInt(CURVE_MODE)){
        case 1:
            m_options->mode1Btn->setChecked(true); break;
        case 2:
            m_options->mode2Btn->setChecked(true); break;
        case 3:
            m_options->mode3Btn->setChecked(true); break;
        default:
            m_options->mode1Btn->setChecked(true); break;
    }
}

KisPropertiesConfiguration*  KisCurvePaintOpSettingsWidget::configuration() const
{
    KisCurvePaintOpSettings* settings = new KisCurvePaintOpSettings();
    settings->setOptionsWidget(const_cast<KisCurvePaintOpSettingsWidget*>(this));
    return settings;
}

void KisCurvePaintOpSettingsWidget::writeConfiguration(KisPropertiesConfiguration* config) const
{
    config->setProperty("paintop", "curvebrush"); // XXX: make this a const id string
    config->setProperty(CURVE_MIN_DISTANCE, m_options->minDistSPBox->value());  
    config->setProperty(CURVE_INTERVAL, m_options->pulseSPBox->value());
    config->setProperty(CURVE_MODE, curveMode());
}

int  KisCurvePaintOpSettingsWidget::curveMode() const
{
    if (m_options->mode1Btn->isChecked()) {
        return 1;
    } else if (m_options->mode2Btn->isChecked()) {
        return 2;
    } else if (m_options->mode3Btn->isChecked()) {
        return 3;
    } else return -1;
}

