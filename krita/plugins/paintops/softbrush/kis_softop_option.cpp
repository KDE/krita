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
#include "kis_softop_option.h"
#include <klocale.h>

#include <QWidget>
#include <QRadioButton>

#include "ui_wdgsoftoptions.h"

class KisSoftOpOptionsWidget: public QWidget, public Ui::WdgSoftOptions
{
public:
    KisSoftOpOptionsWidget(QWidget *parent = 0)
        : QWidget(parent)
    {
        setupUi(this);
    }
};

KisSoftOpOption::KisSoftOpOption()
        : KisPaintOpOption(i18n("Brush size"), false)
{
    m_checkable = false;
    m_options = new KisSoftOpOptionsWidget();
    connect(m_options->diameterSpinBox,SIGNAL(valueChanged(double)),SIGNAL( sigSettingChanged()));
    connect(m_options->spacingSPBox,SIGNAL(valueChanged(double)),SIGNAL( sigSettingChanged()));

    setConfigurationPage(m_options);
}

KisSoftOpOption::~KisSoftOpOption()
{
    // delete m_options; 
}

void KisSoftOpOption::setDiameter ( int diameter )
{
    m_options->diameterSpinBox->setValue( diameter );
}


int KisSoftOpOption::diameter() const
{
    return m_options->diameterSpinBox->value();
}


qreal KisSoftOpOption::end() const
{
    return m_options->endSPBox->value();
}


qreal KisSoftOpOption::start() const
{
    return m_options->startSPBox->value();
}


qreal KisSoftOpOption::spacing() const
{
    return m_options->spacingSPBox->value();
}


void KisSoftOpOption::writeOptionSetting(KisPropertiesConfiguration* setting) const
{
    setting->setProperty( "Soft/diameter", diameter() );
    setting->setProperty( "Soft/spacing", spacing() );
}

void KisSoftOpOption::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    m_options->diameterSpinBox->setValue( setting->getInt("Soft/diameter") );
    m_options->spacingSPBox->setValue( setting->getInt("Soft/spacing") );
}

qreal KisSoftOpOption::sigma() const
{
    return m_options->sigmaSPBox->value();
}
