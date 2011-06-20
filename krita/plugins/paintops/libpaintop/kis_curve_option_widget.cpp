/* This file is part of the KDE project
 * Copyright (C) 2008 Boudewijn Rempt <boud@valdyas.org>
 * Copyright (C) 2009 Sven Langkamp   <sven.langkamp@gmail.com>
 * Copyright (C) 2011 Silvio Heinrich <plassy@web.de>
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

KisCurveOptionWidget::KisCurveOptionWidget(KisCurveOption* curveOption, bool hideSlider)
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
    connect(m_curveOptionWidget->sensorSelector, SIGNAL(highlightedSensorChanged(KisDynamicSensor*)), SLOT(updateCurve(KisDynamicSensor*)));
    connect(m_curveOptionWidget->checkBoxUseSameCurve, SIGNAL(stateChanged(int)), SLOT(transferCurve()));
    
    setLabel(m_curveOptionWidget->label_ymin, curveOption->minimumLabel());
    setLabel(m_curveOptionWidget->label_ymax, curveOption->maximumLabel());
    
    updateSensorCurveLabels(m_curveOptionWidget->sensorSelector->currentHighlighted());
    
    m_curveOptionWidget->slider->setRange(curveOption->minValue(), curveOption->maxValue(), 2);
    m_curveOptionWidget->slider->setValue(curveOption->value());
    
    if(hideSlider)
        m_curveOptionWidget->slider->hide();
    
    connect(m_curveOptionWidget->checkBoxUseCurve, SIGNAL(stateChanged(int))  , SLOT(updateValues()));
    connect(m_curveOptionWidget->slider          , SIGNAL(valueChanged(qreal)), SLOT(updateValues()));
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
    m_curveOptionWidget->sensorSelector->setCurrent(m_curveOption->sensor());
    
    m_curveOptionWidget->checkBoxUseCurve->blockSignals(true);
    m_curveOptionWidget->checkBoxUseCurve->setChecked(m_curveOption->isCurveUsed());
    m_curveOptionWidget->checkBoxUseCurve->blockSignals(false);
    
    m_curveOptionWidget->slider->blockSignals(true);
    m_curveOptionWidget->slider->setValue(m_curveOption->value());
    m_curveOptionWidget->slider->blockSignals(false);
    
    updateCurve(m_curveOption->sensor());
    disableWidgets(!m_curveOption->isCurveUsed());
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
    m_curveOptionWidget->sensorSelector->setCurrentCurve(m_curveOptionWidget->curveWidget->curve(), m_curveOptionWidget->checkBoxUseSameCurve->isChecked());
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

void KisCurveOptionWidget::updateCurve(KisDynamicSensor* sensor)
{
    bool blockSignal = m_curveOptionWidget->curveWidget->blockSignals(true);
    m_curveOptionWidget->curveWidget->setCurve(sensor->curve());
    m_curveOptionWidget->curveWidget->blockSignals(blockSignal);
}

void KisCurveOptionWidget::updateValues()
{
    m_curveOption->setValue(m_curveOptionWidget->slider->value());
    m_curveOption->setCurveUsed(m_curveOptionWidget->checkBoxUseCurve->isChecked());
    disableWidgets(!m_curveOptionWidget->checkBoxUseCurve->isChecked());
    emit sigSettingChanged();
}

void KisCurveOptionWidget::disableWidgets(bool disable)
{
    m_curveOptionWidget->checkBoxUseSameCurve->setDisabled(disable);
    m_curveOptionWidget->curveWidget->setDisabled(disable);
    m_curveOptionWidget->sensorSelector->setDisabled(disable);
}

