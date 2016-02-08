/*
 *  Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
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
#include "DualBrushopOption.h"

#include "ui_WdgDualBrushOptions.h"

class KisDualBrushOpOptionsWidget: public QWidget, public Ui::WdgDualBrushOptions
{
public:
    KisDualBrushOpOptionsWidget(QWidget *parent = 0)
        : QWidget(parent) {
        setupUi(this);
    }
};

KisDualBrushOpOption::KisDualBrushOpOption()
    : KisPaintOpOption(KisPaintOpOption::GENERAL, false)
{
    m_checkable = false;
    m_options = new KisDualBrushOpOptionsWidget();
    m_options->hide();

    setObjectName("KisDualBrushOpOption");

    // initialize values
    m_options->radiusSpinBox->setRange(0, 400);
    m_options->radiusSpinBox->setValue(5);
    m_options->radiusSpinBox->setSuffix(" px");

    connect(m_options->radiusSpinBox, SIGNAL(valueChanged(int)), SLOT(emitSettingChanged()));
    connect(m_options->inkDepletionCHBox, SIGNAL(clicked(bool)), SLOT(emitSettingChanged()));
    connect(m_options->opacity, SIGNAL(clicked(bool)), SLOT(emitSettingChanged()));
    connect(m_options->saturation, SIGNAL(clicked(bool)), SLOT(emitSettingChanged()));
    setConfigurationPage(m_options);
}

KisDualBrushOpOption::~KisDualBrushOpOption()
{
    // delete m_options;
}

int KisDualBrushOpOption::radius() const
{
    return m_options->radiusSpinBox->value();
}


void KisDualBrushOpOption::setRadius(int radius) const
{
    m_options->radiusSpinBox->setValue(radius);
}



bool KisDualBrushOpOption::inkDepletion() const
{
    return m_options->inkDepletionCHBox->isChecked();
}



bool KisDualBrushOpOption::opacity() const
{
    return m_options->opacity->isChecked();
}


bool KisDualBrushOpOption::saturation() const
{
    return m_options->saturation->isChecked();
}


void KisDualBrushOpOption::writeOptionSetting(KisPropertiesConfiguration* setting) const
{
    setting->setProperty(DUALBRUSH_RADIUS, radius());
    setting->setProperty(DUALBRUSH_INK_DEPLETION, inkDepletion());
    setting->setProperty(DUALBRUSH_USE_OPACITY, opacity());
    setting->setProperty(DUALBRUSH_USE_SATURATION, saturation());
}

void KisDualBrushOpOption::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    m_options->radiusSpinBox->setValue(setting->getInt(DUALBRUSH_RADIUS));
    m_options->inkDepletionCHBox->setChecked(setting->getBool(DUALBRUSH_INK_DEPLETION));
    m_options->opacity->setChecked(setting->getBool(DUALBRUSH_USE_OPACITY));
    m_options->saturation->setChecked(setting->getBool(DUALBRUSH_USE_SATURATION));
}


