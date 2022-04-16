/*
 * SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include <QWidget>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

#include <klocalizedstring.h>

#include <kis_slider_spin_box.h>

#include "kis_curve_option_widget.h"
#include "kis_pressure_sharpness_option.h"
#include "kis_pressure_sharpness_option_widget.h"

KisPressureSharpnessOptionWidget::KisPressureSharpnessOptionWidget():
    KisCurveOptionWidget(new KisPressureSharpnessOption(), i18n("0.0"), i18n("1.0"))
{
    setObjectName("KisPressureSharpnessOptionWidget");

    m_alignOutline = new QCheckBox(i18n("Align the brush preview outline to the pixel grid"));
    m_alignOutline->setCheckable(true);
    m_alignOutline->setChecked(false);

    QLabel* thresholdLbl = new QLabel(i18n("Soften edge:"));
    m_softenEdge = new KisSliderSpinBox();
    m_softenEdge->setRange(0, 100);
    m_softenEdge->setValue(0); // Sets old behaviour
    m_softenEdge->setSingleStep(1);

    QHBoxLayout* alignHL = new QHBoxLayout;
    alignHL->setMargin(2);
    alignHL->addWidget(m_alignOutline);

    QHBoxLayout* softnessHL = new QHBoxLayout;
    softnessHL->setMargin(9);
    softnessHL->addWidget(thresholdLbl);
    softnessHL->addWidget(m_softenEdge, 1);

    QWidget* page = new QWidget;
    QVBoxLayout* pageLayout = new QVBoxLayout(page);
    pageLayout->setMargin(0);
    pageLayout->addLayout(alignHL);
    pageLayout->addLayout(softnessHL);
    pageLayout->addWidget(KisCurveOptionWidget::curveWidget());

    connect(m_alignOutline, SIGNAL(toggled(bool)), SLOT(setAlignOutlineToPixels(bool)));
    connect(m_softenEdge, SIGNAL(valueChanged(int)), SLOT(setThreshold(int)));

    setConfigurationPage(page);

    setAlignOutlineToPixels(m_alignOutline->isChecked());
    setThreshold(m_softenEdge->value());
}

void KisPressureSharpnessOptionWidget::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisCurveOptionWidget::readOptionSetting(setting);
    m_alignOutline->setChecked(static_cast<KisPressureSharpnessOption*>(curveOption())->alignOutlineToPixels());
    m_softenEdge->setValue(static_cast<KisPressureSharpnessOption*>(curveOption())->threshold());
}

void KisPressureSharpnessOptionWidget::setAlignOutlineToPixels(bool alignOutline)
{
    static_cast<KisPressureSharpnessOption*>(curveOption())->setAlignOutlineToPixels(alignOutline);
    emitSettingChanged();
}


void KisPressureSharpnessOptionWidget::setThreshold(int threshold)
{
    static_cast<KisPressureSharpnessOption*>(curveOption())->setThreshold(threshold);
    emitSettingChanged();
}
