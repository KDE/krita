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

#include "kis_curve_option_widget.h"

#include "ui_wdgcurveoption.h"
#include "widgets/kis_curve_widget.h"
#include "kis_dynamic_sensor.h"
#include "kis_global.h"
#include "kis_curve_option.h"

KisCurveOptionWidget::KisCurveOptionWidget(KisCurveOption* curveOption)
        : KisPaintOpOption(curveOption->label(), curveOption->isChecked())
        , m_widget(new QWidget)
        , m_curveOptionWidget(new Ui_WdgCurveOption())
        , m_curveOption(curveOption)
{
    m_curveOptionWidget->setupUi(m_widget);
    setConfigurationPage(m_widget);
    connect(m_curveOptionWidget->curveWidget, SIGNAL(modified()), this, SLOT(transferCurve()));
    connect(m_curveOptionWidget->sensorSelector, SIGNAL(sensorChanged(KisDynamicSensor*)), SLOT(setSensor(KisDynamicSensor*)));
    transferCurve();
}

KisCurveOptionWidget::~KisCurveOptionWidget()
{
    delete m_curveOption;
}

void KisCurveOptionWidget::writeOptionSetting(KisPropertiesConfiguration* setting) const
{
    m_curveOption->writeOptionSetting(setting);
}

void KisCurveOptionWidget::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    m_curveOption->readOptionSetting(setting);
    m_curveOptionWidget->curveWidget->setCurve(m_curveOption->curve());
    m_curveOptionWidget->sensorSelector->setCurrent(m_curveOption->sensor());
}

bool KisCurveOptionWidget::isCheckable()
{
    return m_curveOption->isCheckable();
}

bool KisCurveOptionWidget::isChecked() const
{
    return m_curveOption->isChecked();
}

void KisCurveOptionWidget::setChecked(bool checked)
{
    m_curveOption->setChecked(checked);
}

KisCurveOption* KisCurveOptionWidget::curveOption()
{
    return m_curveOption;
}

QWidget* KisCurveOptionWidget::curveWidget()
{
    return m_widget;
}

void KisCurveOptionWidget::transferCurve()
{
    m_curveOption->setCurve(m_curveOptionWidget->curveWidget->curve());

    emit sigSettingChanged();
}

void KisCurveOptionWidget::setSensor(KisDynamicSensor* sensor)
{
    m_curveOption->setSensor(sensor);
    emit sigSettingChanged();
}

