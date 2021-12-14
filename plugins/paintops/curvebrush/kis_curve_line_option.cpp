/*
 *  SPDX-FileCopyrightText: 2011 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
        curvesOpacitySlider->setSingleStep(0.01);
        curvesOpacitySlider->setValue(1.0);
    }
};

KisCurveOpOption::KisCurveOpOption()
    : KisPaintOpOption(i18nc("Brush settings curve value", "Value"), KisPaintOpOption::GENERAL, false)
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


