/*
 * Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
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

    m_threshold = new KisSliderSpinBox();
    m_threshold->setRange(1, 100);
    m_threshold->setValue(40);
    m_threshold->setSingleStep(1);
    m_threshold->setPrefix(i18n("Threshold:"));
    m_threshold->setSuffix(i18n(" px"));

    QHBoxLayout* hl = new QHBoxLayout;
    hl->setContentsMargins(9,9,9,0); // no bottom spacing
    hl->addWidget(m_threshold);

    QVBoxLayout* vl = new QVBoxLayout;
    vl->addLayout(hl);
    vl->addWidget(KisCurveOptionWidget::curveWidget());

    QWidget* w = new QWidget;
    w->setLayout(vl);

    KisCurveOptionWidget::setConfigurationPage(w);

    connect(m_threshold, SIGNAL(valueChanged(int)), this, SLOT(setThreshold(int)));
    setThreshold(m_threshold->value());
}

void KisPressureSharpnessOptionWidget::setThreshold(int threshold)
{
    static_cast<KisPressureSharpnessOption*>(KisCurveOptionWidget::curveOption())->setThreshold(threshold);
    emitSettingChanged();
}
