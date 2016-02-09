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
#include "DualBrushOption.h"

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
    setConfigurationPage(m_options);
}

KisDualBrushOpOption::~KisDualBrushOpOption()
{
    // delete m_options;
}

void KisDualBrushOpOption::writeOptionSetting(KisPropertiesConfiguration* setting) const
{
//    setting->setProperty(DUALBRUSH_RADIUS, radius());
    }

void KisDualBrushOpOption::readOptionSetting(const KisPropertiesConfiguration* setting)
{
//    m_options->radiusSpinBox->setValue(setting->getInt(DUALBRUSH_RADIUS));
    }


