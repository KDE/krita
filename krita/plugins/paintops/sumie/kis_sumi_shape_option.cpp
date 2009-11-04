/*
 *  Copyright (c) 2008,2009 Lukáš Tvrdý <lukast.dev@gmail.com>
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
#include "kis_sumi_shape_option.h"
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

KisSumiShapeOption::KisSumiShapeOption()
        : KisPaintOpOption(i18n("Brush shape"), false)
{
    m_checkable = false;
    m_options = new KisShapeOptionsWidget();

    connect(m_options->oneDimBrushBtn, SIGNAL(toggled(bool)), SIGNAL(sigSettingChanged()));
    connect(m_options->twoDimBrushBtn, SIGNAL(toggled(bool)), SIGNAL(sigSettingChanged()));
    connect(m_options->mousePressureCBox, SIGNAL(toggled(bool)), SIGNAL(sigSettingChanged()));
    connect(m_options->radiusSpinBox, SIGNAL(valueChanged(int)), SIGNAL(sigSettingChanged()));
    connect(m_options->sigmaSpinBox, SIGNAL(valueChanged(double)), SIGNAL(sigSettingChanged()));
    connect(m_options->rndBox, SIGNAL(valueChanged(double)), SIGNAL(sigSettingChanged()));
    connect(m_options->scaleBox, SIGNAL(valueChanged(double)), SIGNAL(sigSettingChanged()));
    connect(m_options->shearBox, SIGNAL(valueChanged(double)), SIGNAL(sigSettingChanged()));

    setConfigurationPage(m_options);
}

KisSumiShapeOption::~KisSumiShapeOption()
{
    // delete m_options;
}


int KisSumiShapeOption::brushDimension() const
{
    if (m_options->oneDimBrushBtn->isChecked()) {
        return 1;
    } else
        return 2;
}


bool KisSumiShapeOption::mousePressure() const
{
    return m_options->mousePressureCBox->isChecked();
}


int KisSumiShapeOption::radius() const
{
    return m_options->radiusSpinBox->value();
}



void KisSumiShapeOption::setRadius(int radius) const
{
    m_options->radiusSpinBox->setValue(radius);
}


double KisSumiShapeOption::sigma() const
{
    return m_options->sigmaSpinBox->value();
}


double KisSumiShapeOption::randomFactor() const
{
    return m_options->rndBox->value();
}

double KisSumiShapeOption::scaleFactor() const
{
    return m_options->scaleBox->value();
}


void KisSumiShapeOption::setScaleFactor(qreal scale) const
{
    m_options->scaleBox->setValue(scale);
}


double KisSumiShapeOption::shearFactor() const
{
    return m_options->shearBox->value();
}

void KisSumiShapeOption::readOptionSetting(const KisPropertiesConfiguration* config)
{
    m_options->radiusSpinBox->setValue(config->getInt("radius"));
    m_options->sigmaSpinBox->setValue(config->getDouble("sigma"));
    int brushDimensions = config->getInt("brush_dimension");
    if (brushDimensions == 1) {
        m_options->oneDimBrushBtn->setChecked(true);
    } else {
        m_options->twoDimBrushBtn->setChecked(true);
    }

    m_options->mousePressureCBox->setChecked(config->getBool("mouse_pressure"));
    m_options->shearBox->setValue(config->getDouble("shear_factor"));
    m_options->rndBox->setValue(config->getDouble("random_factor"));
    m_options->scaleBox->setValue(config->getDouble("scale_factor"));
}


void KisSumiShapeOption::writeOptionSetting(KisPropertiesConfiguration* config) const
{
    config->setProperty("radius", radius());
    config->setProperty("sigma", sigma());
    config->setProperty("brush_dimension", brushDimension());
    config->setProperty("mouse_pressure", mousePressure());
    config->setProperty("shear_factor", shearFactor());
    config->setProperty("random_factor", randomFactor());
    config->setProperty("scale_factor", scaleFactor());
}

