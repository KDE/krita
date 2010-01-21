/*
 * Copyright (c) 2009,2010 Lukáš Tvrdý (lukast.dev@gmail.com)
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
#include "kis_gridop_option.h"
#include <klocale.h>

#include "ui_wdggridoptions.h"

class KisGridOpOptionsWidget: public QWidget, public Ui::WdgGridOptions
{
public:
    KisGridOpOptionsWidget(QWidget *parent = 0)
        : QWidget(parent)
    {
        setupUi(this);
    }
};

KisGridOpOption::KisGridOpOption()
        : KisPaintOpOption(i18n("Brush size"), false)
{
    m_checkable = false;
    m_options = new KisGridOpOptionsWidget();

    connect(m_options->gridWidthSPBox,SIGNAL(valueChanged(int)),SIGNAL(sigSettingChanged()));
    connect(m_options->gridHeightSPBox,SIGNAL(valueChanged(int)),SIGNAL(sigSettingChanged()));
    connect(m_options->divisionLevelSPBox,SIGNAL(valueChanged(int)),SIGNAL(sigSettingChanged()));
    connect(m_options->divisionPressureCHBox,SIGNAL(toggled(bool)),SIGNAL(sigSettingChanged()));
    connect(m_options->scaleDSPBox,SIGNAL(valueChanged(double)),SIGNAL(sigSettingChanged()));
    connect(m_options->vertBorderDSPBox,SIGNAL(valueChanged(double)),SIGNAL(sigSettingChanged()));
    connect(m_options->horizBorderDSPBox,SIGNAL(valueChanged(double)),SIGNAL(sigSettingChanged()));
    connect(m_options->jitterBorderCHBox,SIGNAL(toggled(bool)),SIGNAL(sigSettingChanged()));

    setConfigurationPage(m_options);
}

KisGridOpOption::~KisGridOpOption()
{
    delete m_options; 
}


int KisGridOpOption::divisionLevel() const
{
    return m_options->divisionLevelSPBox->value();
}


int KisGridOpOption::gridWidth() const
{
    return m_options->gridWidthSPBox->value();
}


void KisGridOpOption::setWidth(int width) const
{
    m_options->gridWidthSPBox->setValue(width);
}


int KisGridOpOption::gridHeight() const
{
    return m_options->gridHeightSPBox->value();
}


void KisGridOpOption::setHeight(int height) const
{
    m_options->gridHeightSPBox->setValue(height);
}


bool KisGridOpOption::pressureDivision() const
{
    return m_options->divisionPressureCHBox->isChecked();
}



qreal KisGridOpOption::horizBorder() const
{
    return m_options->vertBorderDSPBox->value();
}


qreal KisGridOpOption::vertBorder() const
{
    return m_options->horizBorderDSPBox->value();
}



bool KisGridOpOption::randomBorder() const
{
    return m_options->jitterBorderCHBox->isChecked();
}


qreal KisGridOpOption::scale() const
{
    return m_options->scaleDSPBox->value();
}

void KisGridOpOption::writeOptionSetting(KisPropertiesConfiguration* setting) const
{
    setting->setProperty( GRID_WIDTH, gridWidth());
    setting->setProperty( GRID_HEIGHT, gridHeight());
    setting->setProperty( GRID_DIVISION_LEVEL, divisionLevel());
    setting->setProperty( GRID_PRESSURE_DIVISION, pressureDivision());
    setting->setProperty( GRID_SCALE, scale());
    setting->setProperty( GRID_VERTICAL_BORDER, vertBorder());
    setting->setProperty( GRID_HORIZONTAL_BORDER, horizBorder());
    setting->setProperty( GRID_RANDOM_BORDER, randomBorder());
}

void KisGridOpOption::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    m_options->gridWidthSPBox->setValue(setting->getInt(GRID_WIDTH));
    m_options->gridHeightSPBox->setValue(setting->getInt(GRID_HEIGHT));
    m_options->divisionLevelSPBox->setValue(setting->getInt(GRID_DIVISION_LEVEL));
    m_options->divisionPressureCHBox->setChecked(setting->getBool(GRID_PRESSURE_DIVISION));
    m_options->scaleDSPBox->setValue(setting->getDouble(GRID_SCALE));
    m_options->vertBorderDSPBox->setValue(setting->getDouble(GRID_VERTICAL_BORDER));
    m_options->horizBorderDSPBox->setValue(setting->getDouble(GRID_HORIZONTAL_BORDER));
    m_options->jitterBorderCHBox->setChecked(setting->getBool(GRID_RANDOM_BORDER));
}
