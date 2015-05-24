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
#include "kis_signals_blocker.h"


inline void setLabel(QLabel* label, const KisCurveLabel& curve_label)
{
    if (curve_label.icon().isNull()) {
        label->setText(curve_label.name());
    }
    else {
        label->setPixmap(QPixmap::fromImage(curve_label.icon()));
    }
}

KisCurveOptionWidget::KisCurveOptionWidget(KisCurveOption* curveOption, const QString &minLabel, const QString &maxLabel, bool hideSlider)
    : KisPaintOpOption(curveOption->category(), curveOption->isChecked())
    , m_widget(new QWidget)
    , m_curveOptionWidget(new Ui_WdgCurveOption())
    , m_curveOption(curveOption)
{
    setObjectName("KisCurveOptionWidget");

    m_curveOptionWidget->setupUi(m_widget);
    setConfigurationPage(m_widget);

    m_curveOptionWidget->sensorSelector->setCurveOption(curveOption);

    updateSensorCurveLabels(m_curveOptionWidget->sensorSelector->currentHighlighted());
    updateCurve(m_curveOptionWidget->sensorSelector->currentHighlighted());

    connect(m_curveOptionWidget->curveWidget, SIGNAL(modified()), this, SLOT(transferCurve()));
    connect(m_curveOptionWidget->sensorSelector, SIGNAL(parametersChanged()), SLOT(emitSettingChanged()));
    connect(m_curveOptionWidget->sensorSelector, SIGNAL(highlightedSensorChanged(KisDynamicSensorSP )), SLOT(updateSensorCurveLabels(KisDynamicSensorSP )));
    connect(m_curveOptionWidget->sensorSelector, SIGNAL(highlightedSensorChanged(KisDynamicSensorSP )), SLOT(updateCurve(KisDynamicSensorSP )));
    connect(m_curveOptionWidget->checkBoxUseSameCurve, SIGNAL(stateChanged(int)), SLOT(transferCurve()));

    m_curveOptionWidget->label_ymin->setText(minLabel);
    m_curveOptionWidget->label_ymax->setText(maxLabel);

    m_curveOptionWidget->slider->setRange(curveOption->minValue(), curveOption->maxValue(), 2);
    m_curveOptionWidget->slider->setValue(curveOption->value());

    if (hideSlider) {
         m_curveOptionWidget->slider->hide();
         m_curveOptionWidget->strengthLabel->hide();
    }


    connect(m_curveOptionWidget->checkBoxUseCurve, SIGNAL(stateChanged(int))  , SLOT(updateValues()));
    connect(m_curveOptionWidget->slider, SIGNAL(valueChanged(qreal)), SLOT(updateValues()));
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
    setting->dump();

    m_curveOption->readOptionSetting(setting);

    m_curveOptionWidget->checkBoxUseCurve->setChecked(m_curveOption->isCurveUsed());
    m_curveOptionWidget->slider->setValue(m_curveOption->value());
    m_curveOptionWidget->checkBoxUseSameCurve->setChecked(m_curveOption->isSameCurveUsed());

    disableWidgets(!m_curveOption->isCurveUsed());

    m_curveOptionWidget->sensorSelector->reload();
    m_curveOptionWidget->sensorSelector->setCurrent(m_curveOption->activeSensors().first());
    updateSensorCurveLabels(m_curveOptionWidget->sensorSelector->currentHighlighted());
    updateCurve(m_curveOptionWidget->sensorSelector->currentHighlighted());
}

bool KisCurveOptionWidget::isCheckable() const
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
    emitSettingChanged();
}

void KisCurveOptionWidget::updateSensorCurveLabels(KisDynamicSensorSP sensor)
{
    if (sensor) {
        m_curveOptionWidget->label_xmin->setText(sensor->minimumLabel());
        m_curveOptionWidget->label_xmax->setText(sensor->maximumLabel());
    }
}

void KisCurveOptionWidget::updateCurve(KisDynamicSensorSP sensor)
{
    if (sensor) {
        bool blockSignal = m_curveOptionWidget->curveWidget->blockSignals(true);
        m_curveOptionWidget->curveWidget->setCurve(sensor->curve());
        m_curveOptionWidget->curveWidget->blockSignals(blockSignal);
    }
}

void KisCurveOptionWidget::updateValues()
{
    m_curveOption->setValue(m_curveOptionWidget->slider->value());
    m_curveOption->setCurveUsed(m_curveOptionWidget->checkBoxUseCurve->isChecked());
    disableWidgets(!m_curveOptionWidget->checkBoxUseCurve->isChecked());
    emitSettingChanged();
}

void KisCurveOptionWidget::disableWidgets(bool disable)
{
    m_curveOptionWidget->checkBoxUseSameCurve->setDisabled(disable);
    m_curveOptionWidget->curveWidget->setDisabled(disable);
    m_curveOptionWidget->sensorSelector->setDisabled(disable);
    m_curveOptionWidget->label_xmax->setDisabled(disable);
    m_curveOptionWidget->label_xmin->setDisabled(disable);
    m_curveOptionWidget->label_ymax->setDisabled(disable);
    m_curveOptionWidget->label_ymin->setDisabled(disable);

}

