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

    QLabel* thresholdLbl = new QLabel(i18n("Soften edge:"));
    m_softenedge = new KisSliderSpinBox();
    m_softenedge->setRange(0, 100);
    m_softenedge->setValue(0); // Sets old behaviour
    m_softenedge->setSingleStep(1);

    QHBoxLayout* hl = new QHBoxLayout;
    hl->setMargin(9);
    hl->addWidget(thresholdLbl);
    hl->addWidget(m_softenedge, 1);

    QVBoxLayout* vl = new QVBoxLayout;
    vl->setMargin(0);
    vl->addLayout(hl);
    vl->addWidget(KisCurveOptionWidget::curveWidget());

    QWidget* w = new QWidget;
    w->setLayout(vl);

    connect(m_softenedge, SIGNAL(valueChanged(int)), SLOT(setThreshold(int)));

    setConfigurationPage(w);

    setThreshold(m_softenedge->value());
}

void KisPressureSharpnessOptionWidget::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisCurveOptionWidget::readOptionSetting(setting);
    m_softenedge->setValue(static_cast<KisPressureSharpnessOption*>(curveOption())->threshold());
}

void KisPressureSharpnessOptionWidget::setThreshold(int threshold)
{
    static_cast<KisPressureSharpnessOption*>(curveOption())->setThreshold(threshold);
    emitSettingChanged();
}
