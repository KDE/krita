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

#include "kis_pressure_sharpness_option_widget.h"

#include <QWidget>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

#include <klocale.h>

#include <kis_slider_spin_box.h>

#include "kis_pressure_sharpness_option.h"

KisPressureSharpnessOptionWidget::KisPressureSharpnessOptionWidget()
    : KisCurveOptionWidget(new KisPressureSharpnessOption())
{
    QWidget* w = new QWidget;
    
    QLabel* sharpnessLbl = new QLabel(i18n("Sharpness"));
    
    m_sharpnessFactor = new KisDoubleSliderSpinBox();
    m_sharpnessFactor->setRange(0.0,1.0,2);
    m_sharpnessFactor->setValue(1.0);
    m_sharpnessFactor->setSingleStep(0.01);
    
    QLabel* thresholdLbl = new QLabel(i18n("Threshold"));
    
    m_threshold = new KisSliderSpinBox();
    m_threshold->setRange(0,100);
    m_threshold->setValue(40);
    m_threshold->setSingleStep(1);

    QGridLayout * gridLayout = new QGridLayout;
    gridLayout->addWidget(sharpnessLbl, 0,0, Qt::AlignRight);
    gridLayout->addWidget(m_sharpnessFactor, 0,1);

    gridLayout->addWidget(thresholdLbl, 1,0, Qt::AlignRight);
    gridLayout->addWidget(m_threshold, 1,1);
    
    gridLayout->setColumnStretch(1,1);
    
    QVBoxLayout* vl = new QVBoxLayout;
    vl->addLayout(gridLayout);
    vl->addWidget(curveWidget());

    w->setLayout(vl);

    setConfigurationPage(w);
    
    connect(m_sharpnessFactor, SIGNAL(valueChanged(qreal)),this, SLOT(setSharpnessFactor(qreal)));
    connect(m_threshold,SIGNAL(valueChanged(int)),this, SLOT(setThreshold(int)));
    
    setSharpnessFactor(m_sharpnessFactor->value());
    setThreshold(m_threshold->value());

}

void KisPressureSharpnessOptionWidget::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    KisCurveOptionWidget::readOptionSetting(setting);
    m_sharpnessFactor->setValue(static_cast<KisPressureSharpnessOption*>(curveOption())->sharpnessFactor());
    m_threshold->setValue(static_cast<KisPressureSharpnessOption*>(curveOption())->threshold());
}


void KisPressureSharpnessOptionWidget::setSharpnessFactor(qreal sharpness)
{
    static_cast<KisPressureSharpnessOption*>(curveOption())->setSharpnessFactor(sharpness);
    emit sigSettingChanged();
}

void KisPressureSharpnessOptionWidget::setThreshold(int threshold)
{
    static_cast<KisPressureSharpnessOption*>(curveOption())->setThreshold(threshold);
    emit sigSettingChanged();
}