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
#include <klocalizedstring.h>

#include "ui_wdggridoptions.h"

class KisGridOpOptionsWidget: public QWidget, public Ui::WdgGridOptions
{
public:
    KisGridOpOptionsWidget(QWidget *parent = 0)
        : QWidget(parent) {
        setupUi(this);
    }
};

KisGridOpOption::KisGridOpOption()
    : KisPaintOpOption(KisPaintOpOption::GENERAL, false)
{
    setObjectName("KisGridOpOption");

    m_checkable = false;
    m_options = new KisGridOpOptionsWidget();

    // initialize slider values
    m_options->gridWidthSPBox->setPrefix(i18n("Grid Width: "));
    m_options->gridWidthSPBox->setRange(1, 999, 0);
    m_options->gridWidthSPBox->setValue(25);
    m_options->gridWidthSPBox->setSuffix(i18n(" px"));
    m_options->gridWidthSPBox->setExponentRatio(3.0);

    m_options->gridHeightSPBox->setPrefix(i18n("Grid Height: "));
    m_options->gridHeightSPBox->setRange(1, 999, 0);
    m_options->gridHeightSPBox->setValue(25);
    m_options->gridHeightSPBox->setSuffix(i18n(" px"));
    m_options->gridHeightSPBox->setExponentRatio(3.0);

    m_options->divisionLevelSPBox->setPrefix(i18n("Division Level: "));
    m_options->divisionLevelSPBox->setRange(0, 25, 0);
    m_options->divisionLevelSPBox->setValue(2);

    m_options->scaleDSPBox->setPrefix(i18n("Scale: "));
    m_options->scaleDSPBox->setRange(0.1, 10.0, 2);
    m_options->scaleDSPBox->setValue(1.0);
    m_options->scaleDSPBox->setExponentRatio(3.0);

    m_options->vertBorderDSPBox->setPrefix(i18n("Vertical: "));
    m_options->vertBorderDSPBox->setSuffix(i18n(" px"));
    m_options->vertBorderDSPBox->setRange(0, 100, 2);
    m_options->vertBorderDSPBox->setValue(0.0);

    m_options->horizBorderDSPBox->setPrefix(i18n("Horizontal: "));
    m_options->vertBorderDSPBox->setSuffix(i18n(" px"));
    m_options->horizBorderDSPBox->setRange(0, 100, 2);
    m_options->horizBorderDSPBox->setValue(0.0);


    connect(m_options->gridWidthSPBox, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->gridHeightSPBox, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->divisionLevelSPBox, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->divisionPressureCHBox, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    connect(m_options->scaleDSPBox, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->vertBorderDSPBox, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->horizBorderDSPBox, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->jitterBorderCHBox, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));

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

void KisGridOpOption::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    KisGridOpProperties op;

    op.grid_width = gridWidth();
    op.grid_height = gridHeight();
    op.grid_division_level = divisionLevel();
    op.grid_pressure_division = pressureDivision();
    op.grid_scale = scale();
    op.grid_vertical_border = vertBorder();
    op.grid_horizontal_border = horizBorder();
    op.grid_random_border = randomBorder();

    op.writeOptionSetting(setting);
}

void KisGridOpOption::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisGridOpProperties op;
    op.readOptionSetting(setting);

    m_options->gridWidthSPBox->setValue(op.grid_width);
    m_options->gridHeightSPBox->setValue(op.grid_height);
    m_options->divisionLevelSPBox->setValue(op.grid_division_level);
    m_options->divisionPressureCHBox->setChecked(op.grid_pressure_division);
    m_options->scaleDSPBox->setValue(op.grid_scale);
    m_options->vertBorderDSPBox->setValue(op.grid_vertical_border);
    m_options->horizBorderDSPBox->setValue(op.grid_horizontal_border);
    m_options->jitterBorderCHBox->setChecked(op.grid_random_border);
}
