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
#include <QWidget>
#include "kis_hsv_option.h"

#include "ui_wdgHsvOptions.h"

class KisHsvOptionsWidget: public QWidget, public Ui::WdgHsvOption
{
public:
    KisHsvOptionsWidget(QWidget *parent = 0)
            : QWidget(parent) {
        setupUi(this);
    }

};


KisHSVOption::KisHSVOption()
        : KisPaintOpOption(i18n("HSV dynamics"), KisPaintOpOption::colorCategory(), false)
{
    m_checkable = true;
    m_options = new KisHsvOptionsWidget();

    connect(m_options->modeHueCBox,SIGNAL(currentIndexChanged(int)),SIGNAL(sigSettingChanged()));
    connect(m_options->inkAmountHue,SIGNAL(valueChanged(double)),SIGNAL( sigSettingChanged()));
    connect(m_options->hueCurve,SIGNAL(modified()),SIGNAL(sigSettingChanged()));

    connect(m_options->modeSaturationCBox,SIGNAL(currentIndexChanged(int)),SIGNAL(sigSettingChanged()));
    connect(m_options->inkAmountSaturation,SIGNAL(valueChanged(double)),SIGNAL( sigSettingChanged()));
    connect(m_options->saturationCurve,SIGNAL(modified()),SIGNAL(sigSettingChanged()));
    
    connect(m_options->modeValueCBox,SIGNAL(currentIndexChanged(int)),SIGNAL(sigSettingChanged()));
    connect(m_options->inkAmountValue,SIGNAL(valueChanged(double)),SIGNAL( sigSettingChanged()));
    connect(m_options->valueCurve,SIGNAL(modified()),SIGNAL(sigSettingChanged()));

    setConfigurationPage(m_options);
}


KisHSVOption::~KisHSVOption()
{
    delete m_options;
}


void KisHSVOption::writeOptionSetting(KisPropertiesConfiguration* setting) const
{
    setting->setProperty( HSV_ENABLED, isChecked() );
    setting->setProperty( HSV_HMODE, m_options->modeHueCBox->currentIndex() );
    setting->setProperty( HSV_HUE_INK_AMOUNT, m_options->inkAmountHue->value());
    setting->setProperty( HSV_HUE_CURVE, qVariantFromValue( m_options->hueCurve->curve() ) );

    setting->setProperty( HSV_SMODE, m_options->modeSaturationCBox->currentIndex() );
    setting->setProperty( HSV_SATURATION_INK_AMOUNT, m_options->inkAmountSaturation->value());
    setting->setProperty( HSV_SATURATION_CURVE, qVariantFromValue( m_options->saturationCurve->curve() ) );
    
    setting->setProperty( HSV_VMODE, m_options->modeValueCBox->currentIndex() );
    setting->setProperty( HSV_VALUE_INK_AMOUNT, m_options->inkAmountValue->value());
    setting->setProperty( HSV_VALUE_CURVE, qVariantFromValue( m_options->valueCurve->curve() ) );
}


void KisHSVOption::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    setChecked(setting->getBool( HSV_ENABLED));
    m_options->inkAmountHue->setValue(setting->getDouble(HSV_HUE_INK_AMOUNT));
    m_options->modeHueCBox->setCurrentIndex(setting->getInt(HSV_HMODE));
    m_options->hueCurve->setCurve(setting->getCubicCurve(HSV_HUE_CURVE));

    m_options->inkAmountSaturation->setValue(setting->getDouble(HSV_SATURATION_INK_AMOUNT));
    m_options->modeSaturationCBox->setCurrentIndex(setting->getInt(HSV_SMODE));
    m_options->saturationCurve->setCurve(setting->getCubicCurve(HSV_SATURATION_CURVE));
    
    m_options->inkAmountValue->setValue(setting->getDouble(HSV_VALUE_INK_AMOUNT));
    m_options->modeValueCBox->setCurrentIndex(setting->getInt(HSV_VMODE));
    m_options->valueCurve->setCurve(setting->getCubicCurve(HSV_VALUE_CURVE));
}



