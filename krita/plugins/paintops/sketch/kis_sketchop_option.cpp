/*
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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
#include "kis_sketchop_option.h"

#include "ui_wdgsketchoptions.h"

class KisSketchOpOptionsWidget: public QWidget, public Ui::WdgSketchOptions
{
public:
    KisSketchOpOptionsWidget(QWidget *parent = 0)
            : QWidget(parent) {
        setupUi(this);
    }
};

KisSketchOpOption::KisSketchOpOption()
        : KisPaintOpOption(i18n("Brush size"), KisPaintOpOption::brushCategory(), false)
{
    m_checkable = false;
    m_options = new KisSketchOpOptionsWidget();
    connect(m_options->radiusKNumInp, SIGNAL(valueChanged(double)), SIGNAL(sigSettingChanged()));
    connect(m_options->offsetSPBox,SIGNAL(valueChanged(double)), SIGNAL(sigSettingChanged()));
    connect(m_options->lowOpacityCHBox,SIGNAL(toggled(bool)),SIGNAL(sigSettingChanged()));
    connect(m_options->connectionCHBox,SIGNAL(toggled(bool)),SIGNAL(sigSettingChanged()));
    connect(m_options->magnetifyCHBox,SIGNAL(toggled(bool)),SIGNAL(sigSettingChanged()));
    connect(m_options->lineWidthSPBox,SIGNAL(valueChanged(double)),SIGNAL(sigSettingChanged()));
    connect(m_options->densitySPBox, SIGNAL(valueChanged(double)),SIGNAL(sigSettingChanged()));
    
    
    setConfigurationPage(m_options);
}

KisSketchOpOption::~KisSketchOpOption()
{
    // delete m_options;
}

void KisSketchOpOption::setThreshold(int radius) const
{
    m_options->radiusKNumInp->setValue(radius);
}

int KisSketchOpOption::threshold() const
{
    return m_options->radiusKNumInp->value();
}


void KisSketchOpOption::writeOptionSetting(KisPropertiesConfiguration* settings) const
{
    settings->setProperty(SKETCH_RADIUS,m_options->radiusKNumInp->value());
    settings->setProperty(SKETCH_OFFSET,m_options->offsetSPBox->value());
    settings->setProperty(SKETCH_PROBABILITY, m_options->densitySPBox->value());
    settings->setProperty(SKETCH_LOW_OPACITY,m_options->lowOpacityCHBox->isChecked());
    settings->setProperty(SKETCH_MAKE_CONNECTION, m_options->connectionCHBox->isChecked());
    settings->setProperty(SKETCH_MAGNETIFY,m_options->magnetifyCHBox->isChecked());
    settings->setProperty(SKETCH_LINE_WIDTH,m_options->lineWidthSPBox->value());
}

void KisSketchOpOption::readOptionSetting(const KisPropertiesConfiguration* settings)
{
        m_options->radiusKNumInp->setValue(settings->getInt(SKETCH_RADIUS));
        m_options->offsetSPBox->setValue( settings->getDouble(SKETCH_OFFSET) );
        m_options->lowOpacityCHBox->setChecked(settings->getBool(SKETCH_LOW_OPACITY));
        m_options->connectionCHBox->setChecked(settings->getBool(SKETCH_MAKE_CONNECTION));
        m_options->magnetifyCHBox->setChecked(settings->getBool(SKETCH_MAGNETIFY));
        m_options->lineWidthSPBox->setValue(settings->getInt(SKETCH_LINE_WIDTH));
        m_options->densitySPBox->setValue(settings->getDouble(SKETCH_PROBABILITY));
}


