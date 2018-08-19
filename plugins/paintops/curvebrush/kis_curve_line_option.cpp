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
        lineWidthSlider->setSuffix(i18n(" px"));

        curvesOpacitySlider->setRange(0.0, 1.0, 2);
        curvesOpacitySlider->setValue(1.0);
    }
};

KisCurveOpOption::KisCurveOpOption()
    : KisPaintOpOption(KisPaintOpOption::GENERAL, false)
{
    m_checkable = false;
    m_options = new KisCurveOpOptionsWidget();

    connect(m_options->connectionCHBox, SIGNAL(toggled(bool)), this, SLOT(emitSettingChanged()));
    connect(m_options->smoothingCHBox, SIGNAL(toggled(bool)), this, SLOT(emitSettingChanged()));
    connect(m_options->historySizeSlider, SIGNAL(valueChanged(qreal)), this, SLOT(emitSettingChanged()));
    connect(m_options->lineWidthSlider, SIGNAL(valueChanged(qreal)), this, SLOT(emitSettingChanged()));
    connect(m_options->curvesOpacitySlider, SIGNAL(valueChanged(qreal)), this, SLOT(emitSettingChanged()));

    setConfigurationPage(m_options);

    setObjectName("KisCurveOpOption");
}

KisCurveOpOption::~KisCurveOpOption()
{
}

void KisCurveOpOption::writeOptionSetting(KisPropertiesConfigurationSP config) const
{
    KisCurveOptionProperties op;

    op.curve_paint_connection_line = m_options->connectionCHBox->isChecked();
    op.curve_smoothing = m_options->smoothingCHBox->isChecked();
    op.curve_stroke_history_size = m_options->historySizeSlider->value();
    op.curve_line_width = m_options->lineWidthSlider->value();
    op.curve_curves_opacity = m_options->curvesOpacitySlider->value();

    op.writeOptionSetting(config);
}

void KisCurveOpOption::readOptionSetting(const KisPropertiesConfigurationSP config)
{
    KisCurveOptionProperties op;
    op.readOptionSetting(config);

    m_options->connectionCHBox->setChecked(op.curve_paint_connection_line);
    m_options->smoothingCHBox->setChecked(op.curve_smoothing);
    m_options->historySizeSlider->setValue(op.curve_stroke_history_size);
    m_options->lineWidthSlider->setValue(op.curve_line_width);
    m_options->curvesOpacitySlider->setValue(op.curve_curves_opacity);
}


