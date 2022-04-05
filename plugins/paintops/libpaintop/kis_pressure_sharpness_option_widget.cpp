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

    m_alignoutline = new QCheckBox(i18n("Align the brush preview outline to the pixel grid"));
    m_alignoutline->setCheckable(true);
    m_alignoutline->setChecked(false);

    QLabel* thresholdLbl = new QLabel(i18n("Soften edge:"));
    m_softenedge = new KisSliderSpinBox();
    m_softenedge->setRange(0, 100);
    m_softenedge->setValue(0); // Sets old behaviour
    m_softenedge->setSingleStep(1);

    QHBoxLayout* align_hl = new QHBoxLayout;
    align_hl->setMargin(2);
    align_hl->addWidget(m_alignoutline);

    QHBoxLayout* softness_hl = new QHBoxLayout;
    softness_hl->setMargin(2);
    softness_hl->addWidget(thresholdLbl);
    softness_hl->addWidget(m_softenedge, 1);

    QWidget* page = new QWidget;
    QVBoxLayout* pageLayout = new QVBoxLayout(page);
    pageLayout->setMargin(0);
    pageLayout->addLayout(align_hl);
    pageLayout->addLayout(softness_hl);
    pageLayout->addWidget(KisCurveOptionWidget::curveWidget());

    connect(m_alignoutline, SIGNAL(toggled(bool)), SLOT(setAlignOutlineToPixels(bool)));
    connect(m_softenedge, SIGNAL(valueChanged(int)), SLOT(setThreshold(int)));

    setConfigurationPage(page);

    setAlignOutlineToPixels(m_alignoutline->isChecked());
    setThreshold(m_softenedge->value());
}

void KisPressureSharpnessOptionWidget::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisCurveOptionWidget::readOptionSetting(setting);
    m_alignoutline->setChecked(static_cast<KisPressureSharpnessOption*>(curveOption())->alignOutlineToPixels());
    m_softenedge->setValue(static_cast<KisPressureSharpnessOption*>(curveOption())->threshold());
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
