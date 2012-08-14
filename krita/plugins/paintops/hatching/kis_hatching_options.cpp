/*
 *  Copyright (c) 2008 Lukas Tvrdy <lukast.dev@gmail.com>
 *  Copyright (c) 2010 José Luis Vergara <pentalis@gmail.com>
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

#include "kis_hatching_options.h"

#include "ui_wdghatchingoptions.h"

class KisHatchingOptionsWidget: public QWidget, public Ui::WdgHatchingOptions
{
public:
    KisHatchingOptionsWidget(QWidget *parent = 0)
            : QWidget(parent) {
        setupUi(this);
        
        QString degree = QChar(Qt::Key_degree);
        QString px = " px";
        
        //setRange(minimum, maximum, decimals)
        
        angleKisDoubleSliderSpinBox     -> setRange(-90.0, 90.0, 1);
        separationKisDoubleSliderSpinBox-> setRange(1.0, 30.0, 1);
        thicknessKisDoubleSliderSpinBox -> setRange(1.0, 30.0, 1);
        originXKisDoubleSliderSpinBox   -> setRange(-300, 300, 0);
        originYKisDoubleSliderSpinBox   -> setRange(-300, 300, 0);
        
        angleKisDoubleSliderSpinBox     -> setValue(-60);
        separationKisDoubleSliderSpinBox-> setValue(6);
        thicknessKisDoubleSliderSpinBox -> setValue(1);
        originXKisDoubleSliderSpinBox   -> setValue(50);
        originYKisDoubleSliderSpinBox   -> setValue(50);
        
        angleKisDoubleSliderSpinBox     -> setSuffix(degree);
        separationKisDoubleSliderSpinBox-> setSuffix(px);
        thicknessKisDoubleSliderSpinBox -> setSuffix(px);
        originXKisDoubleSliderSpinBox   -> setSuffix(px);
        originYKisDoubleSliderSpinBox   -> setSuffix(px);
    }
};

KisHatchingOptions::KisHatchingOptions()
        : KisPaintOpOption(i18n("Hatching options"), KisPaintOpOption::brushCategory(), false)
{
    m_checkable = false;
    m_options = new KisHatchingOptionsWidget();
    
    connect(m_options->angleKisDoubleSliderSpinBox, SIGNAL(valueChanged(qreal)), SIGNAL(sigSettingChanged()));
    connect(m_options->separationKisDoubleSliderSpinBox, SIGNAL(valueChanged(qreal)), SIGNAL(sigSettingChanged()));
    connect(m_options->thicknessKisDoubleSliderSpinBox, SIGNAL(valueChanged(qreal)), SIGNAL(sigSettingChanged()));
    connect(m_options->originXKisDoubleSliderSpinBox, SIGNAL(valueChanged(qreal)), SIGNAL(sigSettingChanged()));
    connect(m_options->originYKisDoubleSliderSpinBox, SIGNAL(valueChanged(qreal)), SIGNAL(sigSettingChanged()));
   
    connect(m_options->noCrosshatchingRadioButton, SIGNAL(clicked(bool)),SIGNAL(sigSettingChanged()));
    connect(m_options->perpendicularRadioButton, SIGNAL(clicked(bool)),SIGNAL(sigSettingChanged()));
    connect(m_options->minusThenPlusRadioButton, SIGNAL(clicked(bool)),SIGNAL(sigSettingChanged()));
    connect(m_options->plusThenMinusRadioButton, SIGNAL(clicked(bool)),SIGNAL(sigSettingChanged()));
    connect(m_options->moirePatternRadioButton, SIGNAL(clicked(bool)),SIGNAL(sigSettingChanged()));
    
    connect(m_options->separationIntervalSpinBox, SIGNAL(valueChanged(int)),SIGNAL(sigSettingChanged()));
    
    setConfigurationPage(m_options);
}

KisHatchingOptions::~KisHatchingOptions()
{
}

void KisHatchingOptions::writeOptionSetting(KisPropertiesConfiguration* setting) const
{
    setting->setProperty("Hatching/angle", m_options->angleKisDoubleSliderSpinBox->value() );
    setting->setProperty("Hatching/separation", m_options->separationKisDoubleSliderSpinBox->value() );
    setting->setProperty("Hatching/thickness", m_options->thicknessKisDoubleSliderSpinBox->value() );
    setting->setProperty("Hatching/origin_x", m_options->originXKisDoubleSliderSpinBox->value() );
    setting->setProperty("Hatching/origin_y", m_options->originYKisDoubleSliderSpinBox->value() );
    
    setting->setProperty("Hatching/bool_nocrosshatching", m_options->noCrosshatchingRadioButton->isChecked() );
    setting->setProperty("Hatching/bool_perpendicular", m_options->perpendicularRadioButton->isChecked() );
    setting->setProperty("Hatching/bool_minusthenplus", m_options->minusThenPlusRadioButton->isChecked() );
    setting->setProperty("Hatching/bool_plusthenminus", m_options->plusThenMinusRadioButton->isChecked() );
    setting->setProperty("Hatching/bool_moirepattern", m_options->moirePatternRadioButton->isChecked() );
    
    setting->setProperty("Hatching/separationintervals", m_options->separationIntervalSpinBox->value() );
}

void KisHatchingOptions::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    m_options->angleKisDoubleSliderSpinBox->setValue( setting->getDouble("Hatching/angle") );
    m_options->separationKisDoubleSliderSpinBox->setValue( setting->getDouble("Hatching/separation")); 
    m_options->thicknessKisDoubleSliderSpinBox->setValue( setting->getDouble("Hatching/thickness") ); 
    m_options->originXKisDoubleSliderSpinBox->setValue( setting->getDouble("Hatching/origin_x") ); 
    m_options->originYKisDoubleSliderSpinBox->setValue( setting->getDouble("Hatching/origin_y") );
    
    m_options->noCrosshatchingRadioButton->setChecked( setting->getBool("Hatching/bool_nocrosshatching") );
    m_options->perpendicularRadioButton->setChecked( setting->getBool("Hatching/bool_perpendicular") );
    m_options->minusThenPlusRadioButton->setChecked( setting->getBool("Hatching/bool_minusthenplus") );
    m_options->plusThenMinusRadioButton->setChecked( setting->getBool("Hatching/bool_plusthenminus") );
    m_options->moirePatternRadioButton->setChecked( setting->getBool("Hatching/bool_moirepattern") );
    
    m_options->separationIntervalSpinBox->setValue( setting->getInt("Hatching/separationintervals") );
}
