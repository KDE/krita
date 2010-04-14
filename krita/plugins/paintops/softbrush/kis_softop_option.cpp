/*
 *  Copyright (c) 2009,2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

KisSoftOpOption::KisSoftOpOption()
        : KisPaintOpOption(i18n("Softness"), false)
{
    m_checkable = false;
    m_options = new KisSoftBrushSelectionWidget();

    connect(m_options->m_gaussBrushTip->endSPBox,SIGNAL(valueChanged(double)),SIGNAL( sigSettingChanged()));
    connect(m_options->m_gaussBrushTip->startSPBox,SIGNAL(valueChanged(double)),SIGNAL( sigSettingChanged()));
    connect(m_options->m_gaussBrushTip->sigmaSPBox,SIGNAL(valueChanged(double)),SIGNAL( sigSettingChanged()));
    connect(m_options->m_gaussBrushTip->flowSPBox,SIGNAL(valueChanged(double)),SIGNAL( sigSettingChanged()));

    connect(m_options->m_curveBrushTip->softCurve, SIGNAL(modified()),SIGNAL(sigSettingChanged()));
    connect(m_options->m_curveBrushTip->pressureCBox, SIGNAL(toggled(bool)),SIGNAL(sigSettingChanged()));
    
    connect(m_options->m_brushesTab, SIGNAL(currentChanged(int)),SIGNAL(sigSettingChanged()));
    
    setConfigurationPage(m_options);
}

KisSoftOpOption::~KisSoftOpOption()
{
    delete m_options; 
}



void KisSoftOpOption::writeOptionSetting(KisPropertiesConfiguration* setting) const
{
    setting->setProperty( SOFT_BRUSH_TIP, m_options->m_brushesTab->currentIndex());
    setting->setProperty( SOFT_END, m_options->m_gaussBrushTip->endSPBox->value() );
    setting->setProperty( SOFT_START, m_options->m_gaussBrushTip->startSPBox->value() );
    setting->setProperty( SOFT_SIGMA, m_options->m_gaussBrushTip->sigmaSPBox->value() );
    setting->setProperty( SOFT_SOFTNESS, m_options->m_gaussBrushTip->flowSPBox->value() );
    
    setting->setProperty( SOFTCURVE_CURVE, qVariantFromValue(m_options->m_curveBrushTip->softCurve->curve()) );
    setting->setProperty( SOFTCURVE_CONTROL_BY_PRESSURE, m_options->m_curveBrushTip->pressureCBox->isChecked() );
}

void KisSoftOpOption::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    if (m_options->m_brushesTab->currentIndex() == 0){
        m_options->setCurveBrush(true);
    }else if (m_options->m_brushesTab->currentIndex() == 1){
        m_options->setGaussianBrush(true);
    }
    
    m_options->m_gaussBrushTip->endSPBox->setValue( setting->getDouble(SOFT_END) );
    m_options->m_gaussBrushTip->startSPBox->setValue( setting->getDouble(SOFT_START) );
    m_options->m_gaussBrushTip->sigmaSPBox->setValue( setting->getDouble(SOFT_SIGMA) );
    m_options->m_gaussBrushTip->flowSPBox->setValue( setting->getDouble(SOFT_SOFTNESS) );
    
    m_options->m_curveBrushTip->softCurve->setCurve( setting->getCubicCurve(SOFTCURVE_CURVE) );
    m_options->m_curveBrushTip->pressureCBox->setChecked( setting->getBool(SOFTCURVE_CONTROL_BY_PRESSURE) );
}

