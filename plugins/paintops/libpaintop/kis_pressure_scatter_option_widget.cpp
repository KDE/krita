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

#include "kis_pressure_scatter_option.h"
#include "kis_pressure_scatter_option_widget.h"


KisPressureScatterOptionWidget::KisPressureScatterOptionWidget()
    : KisCurveOptionWidget(new KisPressureScatterOption(), i18n("0.0"), i18n("1.0"))
{
    QWidget* w = new QWidget;
    m_axisX = new QCheckBox(i18n("X"));
    m_axisX->setChecked(true);
    m_axisY = new QCheckBox(i18n("Y"));
    m_axisY->setChecked(true);

    QLabel* scatterLbl = new QLabel(i18n("Axis:"));

    QHBoxLayout* hl = new QHBoxLayout;
    hl->setContentsMargins(9,9,9,0); // no bottom spacing
    hl->setSpacing(6);
    hl->addWidget(scatterLbl);
    hl->addWidget(m_axisX);
    hl->addWidget(m_axisY);
    hl->addStretch(1); // moves everything to the left

    QVBoxLayout* vl = new QVBoxLayout;
    vl->setMargin(0);
    vl->addLayout(hl);
    vl->addWidget(curveWidget());

    w->setLayout(vl);

    connect(m_axisX, SIGNAL(toggled(bool)), SLOT(xAxisEnabled(bool)));
    connect(m_axisY, SIGNAL(toggled(bool)), SLOT(yAxisEnabled(bool)));

    setConfigurationPage(w);

    xAxisEnabled(m_axisX->isChecked());
    yAxisEnabled(m_axisY->isChecked());
}

void KisPressureScatterOptionWidget::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisCurveOptionWidget::readOptionSetting(setting);
    m_axisX->setChecked(static_cast<KisPressureScatterOption*>(curveOption())->isAxisXEnabled());
    m_axisY->setChecked(static_cast<KisPressureScatterOption*>(curveOption())->isAxisYEnabled());
}


void KisPressureScatterOptionWidget::xAxisEnabled(bool enable)
{
    static_cast<KisPressureScatterOption*>(curveOption())->enableAxisX(enable);
    emitSettingChanged();
}

void KisPressureScatterOptionWidget::yAxisEnabled(bool enable)
{
    static_cast<KisPressureScatterOption*>(curveOption())->enableAxisY(enable);
    emitSettingChanged();
}
