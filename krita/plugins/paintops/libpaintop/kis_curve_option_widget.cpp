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

inline void setLabel(QLabel* label, const KisCurveLabel& curve_label)
{
    if(curve_label.icon().isNull())
    {
        label->setText(curve_label.name());
    } else {
        label->setPixmap(QPixmap::fromImage(curve_label.icon()));
    }
}

KisCurveOptionWidget::KisCurveOptionWidget(KisCurveOption* curveOption)
        : KisPaintOpOption(curveOption->label(), curveOption->category(), curveOption->isChecked())
        , m_widget(new QWidget)
        , m_curveOptionWidget(new Ui_WdgCurveOption())
        , m_curveOption(curveOption)
{
    m_curveOptionWidget->setupUi(m_widget);
    setConfigurationPage(m_widget);
    connect(m_curveOptionWidget->curveWidget, SIGNAL(modified()), this, SLOT(transferCurve()));
    connect(m_curveOptionWidget->sensorSelector, SIGNAL(sensorChanged(KisDynamicSensor*)), SLOT(setSensor(KisDynamicSensor*)));
    connect(m_curveOptionWidget->sensorSelector, SIGNAL(parametersChanged()), SIGNAL(sigSettingChanged()));
    connect(m_curveOptionWidget->sensorSelector, SIGNAL(highlightedSensorChanged(KisDynamicSensor*)), SLOT(updateSensorCurveLabels(KisDynamicSensor*)));
    transferCurve();
    setLabel(m_curveOptionWidget->label_ymin, curveOption->minimumLabel());
    setLabel(m_curveOptionWidget->label_ymax, curveOption->maximumLabel());
    updateSensorCurveLabels(m_curveOptionWidget->sensorSelector->currentHighlighted());
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
    m_curveOptionWidget->curveWidget->setCurve(m_curveOption->sensor()->curve());
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
    m_curveOption->sensor()->setCurve(m_curveOptionWidget->curveWidget->curve());

    emit sigSettingChanged();
}

void KisCurveOptionWidget::setSensor(KisDynamicSensor* sensor)
{
    m_curveOption->setSensor(sensor);
    emit sigSettingChanged();
}

void KisCurveOptionWidget::updateSensorCurveLabels(KisDynamicSensor* sensor)
{
    if(sensor)
    {
        setLabel(m_curveOptionWidget->label_xmin, sensor->minimumLabel());
        setLabel(m_curveOptionWidget->label_xmax, sensor->maximumLabel());
    }
}
