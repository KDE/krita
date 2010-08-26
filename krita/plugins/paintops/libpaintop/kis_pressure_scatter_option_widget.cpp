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

#include "kis_pressure_scatter_option_widget.h"

#include <QWidget>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <klocale.h>

#include "kis_pressure_scatter_option.h"
#include <kis_slider_spin_box.h>
#include <QLabel>


KisPressureScatterOptionWidget::KisPressureScatterOptionWidget()
    : KisCurveOptionWidget(new KisPressureScatterOption())
{
    QWidget* w = new QWidget;
    m_axisX = new QCheckBox(i18n("Axis X"));
    m_axisX->setChecked(true);
    m_axisY = new QCheckBox(i18n("Axis Y"));
    m_axisY->setChecked(true);
    
    QLabel* scatterLbl = new QLabel(i18n("Scatter amount"));
    
    m_scatterAmount = new KisDoubleSliderSpinBox();
    m_scatterAmount->setRange(0.0,5.0,2);
    m_scatterAmount->setValue(1.0);
    m_scatterAmount->setSingleStep(0.1);
    
    
    QHBoxLayout * scatterHl = new QHBoxLayout;
    scatterHl->addWidget(scatterLbl);
    scatterHl->addWidget(m_scatterAmount);
    scatterHl->setStretch(1,1);
    
    QHBoxLayout* hl = new QHBoxLayout;
    hl->addWidget(m_axisX);
    hl->addWidget(m_axisY);
    
    QVBoxLayout* vl = new QVBoxLayout;
    vl->addLayout(hl);
    vl->addLayout(scatterHl);
    vl->addWidget(curveWidget());

    w->setLayout(vl);

    connect(m_axisX, SIGNAL(toggled(bool)),SLOT(xAxisEnabled(bool)));
    connect(m_axisY, SIGNAL(toggled(bool)),SLOT(yAxisEnabled(bool)));
    connect(m_scatterAmount, SIGNAL(valueChanged(qreal)),SLOT(scatterAmountChanged(qreal)));

    setConfigurationPage(w);
    
    xAxisEnabled(m_axisX->isChecked());
    yAxisEnabled(m_axisY->isChecked());
    scatterAmountChanged(m_scatterAmount->value());
}

void KisPressureScatterOptionWidget::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    KisCurveOptionWidget::readOptionSetting(setting);
    m_axisY->setChecked(static_cast<KisPressureScatterOption*>(curveOption())->isAxisXEnabled());
    m_axisX->setChecked(static_cast<KisPressureScatterOption*>(curveOption())->isAxisYEnabled());
    m_scatterAmount->setValue(static_cast<KisPressureScatterOption*>(curveOption())->scatterAmount());
}


void KisPressureScatterOptionWidget::xAxisEnabled(bool enable)
{
    static_cast<KisPressureScatterOption*>(curveOption())->enableAxisX(enable);
    emit sigSettingChanged();
}

void KisPressureScatterOptionWidget::yAxisEnabled(bool enable)
{
    static_cast<KisPressureScatterOption*>(curveOption())->enableAxisY(enable);
    emit sigSettingChanged();
}

void KisPressureScatterOptionWidget::scatterAmountChanged(qreal value)
{
    static_cast<KisPressureScatterOption*>(curveOption())->setScatterAmount(value);
    emit sigSettingChanged();
}


