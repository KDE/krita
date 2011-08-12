/*
 *  Copyright (c) 2011 Lukáš Tvrdý <lukast.dev@gmail.com>
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
#include "kis_curve_line_option.h"

#include "ui_wdgcurveoptions.h"

class KisCurveOpOptionsWidget: public QWidget, public Ui::WdgCurveOptions
{

public:
    KisCurveOpOptionsWidget(QWidget *parent = 0) : QWidget(parent) {
        setupUi(this);
        historySizeSlider->setRange(2, 300);
        historySizeSlider->setValue(30);

        lineWidthSlider->setRange(1, 100);
        lineWidthSlider->setValue(1);
        lineWidthSlider->setSuffix("px");

        curvesOpacitySlider->setRange(0.0, 1.0, 2);
        curvesOpacitySlider->setValue(1.0);
    }
};

KisCurveOpOption::KisCurveOpOption()
        : KisPaintOpOption(i18n("Lines"), KisPaintOpOption::brushCategory(), false)
{
    m_checkable = false;
    m_options = new KisCurveOpOptionsWidget();

    connect(m_options->connectionCHBox, SIGNAL(toggled(bool)), this, SIGNAL(sigSettingChanged()));
    connect(m_options->smoothingCHBox, SIGNAL(toggled(bool)), this, SIGNAL(sigSettingChanged()));
    connect(m_options->historySizeSlider, SIGNAL(valueChanged(qreal)), this, SIGNAL(sigSettingChanged()));
    connect(m_options->lineWidthSlider, SIGNAL(valueChanged(qreal)), this, SIGNAL(sigSettingChanged()));
    connect(m_options->curvesOpacitySlider, SIGNAL(valueChanged(qreal)), this, SIGNAL(sigSettingChanged()));

    setConfigurationPage(m_options);
}

KisCurveOpOption::~KisCurveOpOption()
{
}

void KisCurveOpOption::writeOptionSetting(KisPropertiesConfiguration* config) const
{
    config->setProperty(CURVE_PAINT_CONNECTION_LINE, m_options->connectionCHBox->isChecked());
    config->setProperty(CURVE_SMOOTHING, m_options->smoothingCHBox->isChecked());
    config->setProperty(CURVE_STROKE_HISTORY_SIZE, m_options->historySizeSlider->value());
    config->setProperty(CURVE_LINE_WIDTH, m_options->lineWidthSlider->value());
    config->setProperty(CURVE_CURVES_OPACITY, m_options->curvesOpacitySlider->value());
}

void KisCurveOpOption::readOptionSetting(const KisPropertiesConfiguration* config)
{
    m_options->connectionCHBox->setChecked(config->getBool(CURVE_PAINT_CONNECTION_LINE));
    m_options->smoothingCHBox->setChecked(config->getBool(CURVE_SMOOTHING));
    m_options->historySizeSlider->setValue(config->getInt(CURVE_STROKE_HISTORY_SIZE));
    m_options->lineWidthSlider->setValue(config->getInt(CURVE_LINE_WIDTH));
    m_options->curvesOpacitySlider->setValue(config->getDouble(CURVE_CURVES_OPACITY));
}


