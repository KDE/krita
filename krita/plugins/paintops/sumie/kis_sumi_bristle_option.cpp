/*
 *  Copyright (c) 2008-2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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
#include "kis_sumi_bristle_option.h"
#include <klocale.h>

#include "ui_wdgbristleoptions.h"

class KisBristleOptionsWidget: public QWidget, public Ui::WdgBristleOptions
{
public:
    KisBristleOptionsWidget(QWidget *parent = 0)
            : QWidget(parent) {
        setupUi(this);
    }
};

KisSumiBristleOption::KisSumiBristleOption()
        : KisPaintOpOption(i18n("Bristle options"), false)
{
    m_checkable = false;
    m_options = new KisBristleOptionsWidget();

    connect(m_options->mousePressureCBox, SIGNAL(toggled(bool)), SIGNAL(sigSettingChanged()));
    connect(m_options->rndBox, SIGNAL(valueChanged(double)), SIGNAL(sigSettingChanged()));
    connect(m_options->scaleBox, SIGNAL(valueChanged(double)), SIGNAL(sigSettingChanged()));
    connect(m_options->shearBox, SIGNAL(valueChanged(double)), SIGNAL(sigSettingChanged()));

    setConfigurationPage(m_options);
}

KisSumiBristleOption::~KisSumiBristleOption()
{
    delete m_options;
}



void KisSumiBristleOption::readOptionSetting(const KisPropertiesConfiguration* config)
{
    m_options->mousePressureCBox->setChecked(config->getBool(SUMI_BRISTLE_USE_MOUSEPRESSURE));
    m_options->shearBox->setValue(config->getDouble(SUMI_BRISTLE_SHEAR));
    m_options->rndBox->setValue(config->getDouble(SUMI_BRISTLE_RANDOM));
    m_options->scaleBox->setValue(config->getDouble(SUMI_BRISTLE_SCALE));
}


void KisSumiBristleOption::writeOptionSetting(KisPropertiesConfiguration* config) const
{
    config->setProperty(SUMI_BRISTLE_USE_MOUSEPRESSURE,m_options->mousePressureCBox->isChecked());
    config->setProperty(SUMI_BRISTLE_SCALE,m_options->scaleBox->value());
    config->setProperty(SUMI_BRISTLE_SHEAR,m_options->shearBox->value());
    config->setProperty(SUMI_BRISTLE_RANDOM,m_options->rndBox->value());
}

