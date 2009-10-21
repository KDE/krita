/*
 *  Copyright (c) 2008 Lukáš Tvrdý <lukast.dev@gmail.com>
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
}

KisCurvePaintOpSettingsWidget::~ KisCurvePaintOpSettingsWidget()
{
}

void  KisCurvePaintOpSettingsWidget::setConfiguration(const KisPropertiesConfiguration * config)
{
    m_options->minDistSPBox->setValue(config->getInt("min_distance"));
    m_options->pulseSPBox->setValue(config->getInt("interval"));

    int curveAction = config->getInt("curve_action");
    if (curveAction == 1) {
        m_options->mode1Btn->setChecked(true);
    } else if (curveAction == 2) {
        m_options->mode2Btn->setChecked(true);
    } else if (curveAction == 3) {
        m_options->mode3Btn->setChecked(true);
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
    config->setProperty("min_distance", minimalDistance());
    config->setProperty("curve_action", curveAction());
    config->setProperty("interval", interval());
}

int  KisCurvePaintOpSettingsWidget::minimalDistance() const
{
    return m_options->minDistSPBox->value();
}

int  KisCurvePaintOpSettingsWidget::interval() const
{
    return m_options->pulseSPBox->value();
}


int  KisCurvePaintOpSettingsWidget::curveAction() const
{
    if (m_options->mode1Btn->isChecked()) {
        return 1;
    } else if (m_options->mode2Btn->isChecked()) {
        return 2;
    } else if (m_options->mode3Btn->isChecked()) {
        return 3;
    } else return -1;
}

