/*
 *  Copyright (c) 2008 Lukas Tvrdy <lukast.dev@gmail.com>
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
#include "kis_dynaop_option.h"
#include <klocale.h>

#include <QWidget>
#include <QRadioButton>

#include "ui_wdgdynaoptions.h"

class KisDynaOpOptionsWidget: public QWidget, public Ui::WdgDynaOptions
{
public:
    KisDynaOpOptionsWidget(QWidget *parent = 0)
            : QWidget(parent) {
        setupUi(this);
    }
};

KisDynaOpOption::KisDynaOpOption()
        : KisPaintOpOption(i18n("Brush size"), false)
{
    m_checkable = false;
    m_options = new KisDynaOpOptionsWidget();
    setConfigurationPage(m_options);
}

KisDynaOpOption::~KisDynaOpOption()
{
    // delete m_options;
}

int KisDynaOpOption::radius() const
{
    return m_options->radiusSpinBox->value();
}

void KisDynaOpOption::setRadius(int radius)
{
    m_options->radiusSpinBox->setValue(radius);
}

void KisDynaOpOption::writeOptionSetting(KisPropertiesConfiguration* setting) const
{
    setting->setProperty("Dyna/radius", radius());
}

void KisDynaOpOption::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    m_options->radiusSpinBox->setValue(setting->getInt("Dyna/radius"));
}

