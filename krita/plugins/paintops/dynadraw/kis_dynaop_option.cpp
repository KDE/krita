/*
 *  Copyright (c) 2009-2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "ui_wdgdynaoptions.h"

class KisDynaOpOptionsWidget: public QWidget, public Ui::WdgDynaOptions
{
public:
    KisDynaOpOptionsWidget(QWidget *parent = 0)
            : QWidget(parent) {
        setupUi(this);
        angleSlider->setRange(0,360,0);
        angleSlider->setValue(0);
        angleSlider->setSingleStep(1);
        angleSlider->setSuffix(QChar(Qt::Key_degree));
        
        diameterDSSB->setRange(0,1000,0);
        diameterDSSB->setValue(20);
        diameterDSSB->setExponentRatio(3.0);
        
    }
};

KisDynaOpOption::KisDynaOpOption()
        : KisPaintOpOption(i18n("Brush size"), KisPaintOpOption::brushCategory(), false)
{
    m_checkable = false;
    m_options = new KisDynaOpOptionsWidget();
    
    //ui 
    connect(m_options->fixedAngleChBox,SIGNAL(toggled(bool)),m_options->angleSlider,SLOT(setEnabled(bool)));
    
    // preset
    connect(m_options->circleRBox,SIGNAL(toggled(bool)),SIGNAL(sigSettingChanged()));
    connect(m_options->polygonRBox,SIGNAL(toggled(bool)),SIGNAL(sigSettingChanged()));
    connect(m_options->wireRBox,SIGNAL(toggled(bool)),SIGNAL(sigSettingChanged()));
    connect(m_options->linesRBox,SIGNAL(toggled(bool)),SIGNAL(sigSettingChanged()));
    connect(m_options->initWidthSPBox,SIGNAL(valueChanged(double)),SIGNAL(sigSettingChanged()));
    connect(m_options->massSPBox,SIGNAL(valueChanged(double)),SIGNAL(sigSettingChanged()));
    connect(m_options->dragSPBox,SIGNAL(valueChanged(double)),SIGNAL(sigSettingChanged()));
    connect(m_options->angleSlider,SIGNAL(valueChanged(qreal)),SIGNAL(sigSettingChanged()));
    connect(m_options->widthRangeSPBox,SIGNAL(valueChanged(double)),SIGNAL(sigSettingChanged()));
    connect(m_options->diameterDSSB,SIGNAL(valueChanged(qreal)),SIGNAL(sigSettingChanged()));
    connect(m_options->lineCountSPBox,SIGNAL(valueChanged(int)),SIGNAL(sigSettingChanged()));
    connect(m_options->lineSpacingSPBox,SIGNAL(valueChanged(double)),SIGNAL(sigSettingChanged()));
    connect(m_options->LineCBox,SIGNAL(clicked(bool)),SIGNAL(sigSettingChanged()));
    connect(m_options->twoCBox,SIGNAL(clicked(bool)),SIGNAL(sigSettingChanged()));
    connect(m_options->fixedAngleChBox,SIGNAL(clicked(bool)),SIGNAL(sigSettingChanged()));
    
    setConfigurationPage(m_options);
}

KisDynaOpOption::~KisDynaOpOption()
{
    delete m_options;
}

qreal KisDynaOpOption::initWidth() const
{
    return m_options->initWidthSPBox->value();
}

qreal KisDynaOpOption::mass() const
{
    return m_options->massSPBox->value();
}

qreal KisDynaOpOption::drag() const
{
    return m_options->dragSPBox->value();
}

bool KisDynaOpOption::useFixedAngle() const
{
    return m_options->fixedAngleChBox->isChecked();
}


qreal KisDynaOpOption::widthRange() const
{
    return m_options->widthRangeSPBox->value();
}


int KisDynaOpOption::action() const
{
    if (m_options->circleRBox->isChecked())
        return 0;
    if (m_options->polygonRBox->isChecked())
        return 1;
    if (m_options->wireRBox->isChecked())
        return 2;
    if (m_options->linesRBox->isChecked())
        return 3;
    return 0;
}


bool KisDynaOpOption::enableLine() const
{
    return m_options->LineCBox->isChecked();
}

bool KisDynaOpOption::useTwoCircles() const
{
    return m_options->twoCBox->isChecked();
}

int KisDynaOpOption::lineCount() const
{
    return m_options->lineCountSPBox->value();
}

qreal KisDynaOpOption::lineSpacing() const
{
    return m_options->lineSpacingSPBox->value();
}

void KisDynaOpOption::writeOptionSetting(KisPropertiesConfiguration* setting) const
{
    setting->setProperty(DYNA_WIDTH, initWidth());
    setting->setProperty(DYNA_MASS ,mass());
    setting->setProperty(DYNA_DRAG ,drag());
    setting->setProperty(DYNA_USE_FIXED_ANGLE ,useFixedAngle());
    setting->setProperty(DYNA_ANGLE ,m_options->angleSlider->value());
    setting->setProperty(DYNA_WIDTH_RANGE ,widthRange());
    setting->setProperty(DYNA_ACTION ,action());
    setting->setProperty(DYNA_DIAMETER ,m_options->diameterDSSB->value());
    setting->setProperty(DYNA_ENABLE_LINE ,enableLine());
    setting->setProperty(DYNA_USE_TWO_CIRCLES ,useTwoCircles());
    setting->setProperty(DYNA_LINE_COUNT ,lineCount());
    setting->setProperty(DYNA_LINE_SPACING ,lineSpacing());

}

void KisDynaOpOption::readOptionSetting(const KisPropertiesConfiguration* setting){
    switch (setting->getInt(DYNA_ACTION))
    {
        case 0: m_options->circleRBox->setChecked(true); break;
        case 1: m_options->polygonRBox->setChecked(true); break;
        case 2: m_options->wireRBox->setChecked(true); break;
        case 3: m_options->linesRBox->setChecked(true); break;
        default: break;
    }
    
    m_options->initWidthSPBox->setValue( setting->getDouble(DYNA_WIDTH));
    m_options->massSPBox->setValue( setting->getDouble(DYNA_MASS));
    m_options->dragSPBox->setValue( setting->getDouble(DYNA_DRAG));
    m_options->angleSlider->setValue( setting->getDouble(DYNA_ANGLE));
    m_options->widthRangeSPBox->setValue( setting->getDouble(DYNA_WIDTH_RANGE));
    m_options->diameterDSSB->setValue( setting->getInt(DYNA_DIAMETER));
    m_options->lineCountSPBox->setValue( setting->getInt(DYNA_LINE_COUNT));
    m_options->lineSpacingSPBox->setValue( setting->getDouble(DYNA_LINE_SPACING));
    m_options->LineCBox->setChecked(setting->getBool(DYNA_ENABLE_LINE));
    m_options->twoCBox->setChecked(setting->getBool(DYNA_USE_TWO_CIRCLES));
    m_options->fixedAngleChBox->setChecked(setting->getBool(DYNA_USE_FIXED_ANGLE));
}
