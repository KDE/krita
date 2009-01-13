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
#include "kis_sprayop_option.h"
#include <klocale.h>

#include <QWidget>
#include <QRadioButton>

#include "ui_wdgsprayoptions.h"

class KisSprayOpOptionsWidget: public QWidget, public Ui::WdgSprayOptions
{
public:
    KisSprayOpOptionsWidget(QWidget *parent = 0)
        : QWidget(parent)
    {
        setupUi(this);
    }
};

KisSprayOpOption::KisSprayOpOption()
        : KisPaintOpOption(i18n("Brush size"), false)
{
    m_checkable = false;
    m_options = new KisSprayOpOptionsWidget();
    setConfigurationPage(m_options);
}

KisSprayOpOption::~KisSprayOpOption()
{
    // delete m_options; 
}

int KisSprayOpOption::radius() const
{
    return m_options->radiusSpinBox->value();
}

void KisSprayOpOption::setRadius( int radius )
{
    m_options->radiusSpinBox->setValue(radius);
}

void KisSprayOpOption::writeOptionSetting(KisPropertiesConfiguration* setting) const
{
    setting->setProperty( "Spray/radius", radius() );
}

void KisSprayOpOption::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    m_options->radiusSpinBox->setValue( setting->getInt("Spray/radius") );
}

