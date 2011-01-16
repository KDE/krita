/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2008
 * Copyright (C) Sven Langkamp <sven.langkamp@gmail.com>, (C) 2009
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

#include "kis_pressure_rate_option_widget.h"
#include "kis_pressure_rate_option.h"
#include <kis_slider_spin_box.h>

#include <QWidget>
#include <QCheckBox>
#include <QLabel>
#include <QSlider>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <klocale.h>

KisPressureRateOptionWidget::KisPressureRateOptionWidget(const QString& label, const QString& sliderLabel, const QString& name, bool checked)
    : KisCurveOptionWidget(new KisPressureRateOption(name, label, checked))
{
    QWidget* w = new QWidget;
    QLabel* rateLabel = new QLabel(sliderLabel);
    m_rateSlider = new KisDoubleSliderSpinBox();
    m_rateSlider->setRange(0.0, 1.0, 2);
    m_rateSlider->setSingleStep(0.01);
    m_rateSlider->setValue(0.3);
    
    connect(m_rateSlider, SIGNAL(valueChanged(qreal)),SLOT(rateChanged(qreal)));
    
    QHBoxLayout* hl = new QHBoxLayout;
    hl->addWidget(rateLabel);
    hl->addWidget(m_rateSlider);
    hl->setStretchFactor(m_rateSlider, 1);

    QVBoxLayout* vl = new QVBoxLayout;
    vl->addLayout(hl);
    vl->addWidget(curveWidget());

    w->setLayout(vl);
    setConfigurationPage(w);
    rateChanged(m_rateSlider->value());
}

void KisPressureRateOptionWidget::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    KisCurveOptionWidget::readOptionSetting(setting);
    m_rateSlider->setValue(static_cast<KisPressureRateOption*>(curveOption())->getRate());
}

void KisPressureRateOptionWidget::rateChanged(qreal rate)
{
    static_cast<KisPressureRateOption*>(curveOption())->setRate(rate);
    emit sigSettingChanged();
}
