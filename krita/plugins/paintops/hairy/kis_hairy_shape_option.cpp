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
#include "kis_hairy_shape_option.h"
#include <klocale.h>

#include "ui_wdgshapeoptions.h"

class KisShapeOptionsWidget: public QWidget, public Ui::WdgShapeOptions
{
public:
    KisShapeOptionsWidget(QWidget *parent = 0)
            : QWidget(parent) {
        setupUi(this);
    }
};

KisHairyShapeOption::KisHairyShapeOption()
        : KisPaintOpOption(i18n("Brush shape"), KisPaintOpOption::brushCategory(), false)
{
    m_checkable = false;
    m_options = new KisShapeOptionsWidget();

    connect(m_options->oneDimBrushBtn, SIGNAL(toggled(bool)), SIGNAL(sigSettingChanged()));
    connect(m_options->twoDimBrushBtn, SIGNAL(toggled(bool)), SIGNAL(sigSettingChanged()));
    connect(m_options->radiusSpinBox, SIGNAL(valueChanged(int)), SIGNAL(sigSettingChanged()));
    connect(m_options->sigmaSpinBox, SIGNAL(valueChanged(double)), SIGNAL(sigSettingChanged()));

    setConfigurationPage(m_options);
}

KisHairyShapeOption::~KisHairyShapeOption()
{
    delete m_options;
}


int KisHairyShapeOption::radius() const
{
    return m_options->radiusSpinBox->value();
}

void KisHairyShapeOption::setRadius(int radius) const
{
    m_options->radiusSpinBox->setValue(radius);
}


void KisHairyShapeOption::readOptionSetting(const KisPropertiesConfiguration* config)
{
    m_options->radiusSpinBox->setValue(config->getInt(HAIRY_RADIUS));
    m_options->sigmaSpinBox->setValue(config->getDouble(HAIRY_SIGMA));
    if (config->getBool(HAIRY_IS_DIMENSION_1D)) {
        m_options->oneDimBrushBtn->setChecked(true);
    } else {
        m_options->twoDimBrushBtn->setChecked(true);
    }
}


void KisHairyShapeOption::writeOptionSetting(KisPropertiesConfiguration* config) const
{
    config->setProperty(HAIRY_RADIUS,radius());
    config->setProperty(HAIRY_SIGMA,m_options->sigmaSpinBox->value());
    config->setProperty(HAIRY_IS_DIMENSION_1D,m_options->oneDimBrushBtn->isChecked()); 
}

