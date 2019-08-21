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
#include "kis_icon_utils.h"

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

    connect(m_curveOptionWidget->curveWidget, SIGNAL(modified()), this, SLOT(slotModified()));
    connect(m_curveOptionWidget->sensorSelector, SIGNAL(parametersChanged()), SLOT(emitSettingChanged()));
    connect(m_curveOptionWidget->sensorSelector, SIGNAL(parametersChanged()), SLOT(updateLabelsOfCurrentSensor()));
    connect(m_curveOptionWidget->sensorSelector, SIGNAL(highlightedSensorChanged(KisDynamicSensorSP)), SLOT(updateSensorCurveLabels(KisDynamicSensorSP)));
    connect(m_curveOptionWidget->sensorSelector, SIGNAL(highlightedSensorChanged(KisDynamicSensorSP)), SLOT(updateCurve(KisDynamicSensorSP)));
    connect(m_curveOptionWidget->checkBoxUseSameCurve, SIGNAL(stateChanged(int)), SLOT(slotStateChanged()));


    // set all the icons for the curve preset shapes
    updateThemedIcons();

    // various curve preset buttons with predefined curves
    connect(m_curveOptionWidget->linearCurveButton, SIGNAL(clicked(bool)), this, SLOT(changeCurveLinear()));
    connect(m_curveOptionWidget->revLinearButton, SIGNAL(clicked(bool)), this, SLOT(changeCurveReverseLinear()));
    connect(m_curveOptionWidget->jCurveButton, SIGNAL(clicked(bool)), this, SLOT(changeCurveJShape()));
    connect(m_curveOptionWidget->lCurveButton, SIGNAL(clicked(bool)), this, SLOT(changeCurveLShape()));
    connect(m_curveOptionWidget->sCurveButton, SIGNAL(clicked(bool)), this, SLOT(changeCurveSShape()));
    connect(m_curveOptionWidget->reverseSCurveButton, SIGNAL(clicked(bool)), this, SLOT(changeCurveReverseSShape()));
    connect(m_curveOptionWidget->uCurveButton, SIGNAL(clicked(bool)), this, SLOT(changeCurveUShape()));
    connect(m_curveOptionWidget->revUCurveButton, SIGNAL(clicked(bool)), this, SLOT(changeCurveArchShape()));


    m_curveOptionWidget->label_ymin->setText(minLabel);
    m_curveOptionWidget->label_ymax->setText(maxLabel);

    // strength settings is shown as 0-100%
    m_curveOptionWidget->strengthSlider->setRange(curveOption->minValue()*100, curveOption->maxValue()*100, 0);
    m_curveOptionWidget->strengthSlider->setValue(curveOption->value()*100);
    m_curveOptionWidget->strengthSlider->setPrefix(i18n("Strength: "));
    m_curveOptionWidget->strengthSlider->setSuffix(i18n("%"));


    if (hideSlider) {
         m_curveOptionWidget->strengthSlider->hide();
    }

    connect(m_curveOptionWidget->checkBoxUseCurve, SIGNAL(stateChanged(int))  , SLOT(updateValues()));
    connect(m_curveOptionWidget->curveMode, SIGNAL(currentIndexChanged(int)), SLOT(updateMode()));
    connect(m_curveOptionWidget->strengthSlider, SIGNAL(valueChanged(qreal)), SLOT(updateValues()));
}

KisCurveOptionWidget::~KisCurveOptionWidget()
{
    delete m_curveOption;
    delete m_curveOptionWidget;
}

void KisCurveOptionWidget::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    m_curveOption->writeOptionSetting(setting);
}

void KisCurveOptionWidget::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    //setting->dump();

    m_curveOption->readOptionSetting(setting);

    m_curveOptionWidget->checkBoxUseCurve->setChecked(m_curveOption->isCurveUsed());
    m_curveOptionWidget->strengthSlider->setValue(m_curveOption->value()*100);
    m_curveOptionWidget->checkBoxUseSameCurve->setChecked(m_curveOption->isSameCurveUsed());
    m_curveOptionWidget->curveMode->setCurrentIndex(m_curveOption->getCurveMode());

    disableWidgets(!m_curveOption->isCurveUsed());

    m_curveOptionWidget->sensorSelector->reload();
    m_curveOptionWidget->sensorSelector->setCurrent(m_curveOption->activeSensors().first());
    updateSensorCurveLabels(m_curveOptionWidget->sensorSelector->currentHighlighted());
    updateCurve(m_curveOptionWidget->sensorSelector->currentHighlighted());

    if (m_curveOption->isSameCurveUsed()) {
        // make sure the curve is transfered to all sensors to avoid updating from a wrong curve later
        transferCurve();
    }
}

void KisCurveOptionWidget::lodLimitations(KisPaintopLodLimitations *l) const
{
    m_curveOption->lodLimitations(l);
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

void KisCurveOptionWidget::slotModified()
{
    transferCurve();
}

void KisCurveOptionWidget::slotStateChanged()
{
    transferCurve();
}


void KisCurveOptionWidget::transferCurve()
{
    m_curveOptionWidget->sensorSelector->setCurrentCurve(m_curveOptionWidget->curveWidget->curve(), m_curveOptionWidget->checkBoxUseSameCurve->isChecked());
    emitSettingChanged();
}

void KisCurveOptionWidget::updateSensorCurveLabels(KisDynamicSensorSP sensor)
{
    if (sensor) {
        m_curveOptionWidget->label_xmin->setText(KisDynamicSensor::minimumLabel(sensor->sensorType()));
        m_curveOptionWidget->label_xmax->setText(KisDynamicSensor::maximumLabel(sensor->sensorType(), sensor->length()));

        int inMinValue = KisDynamicSensor::minimumValue(sensor->sensorType());
        int inMaxValue = KisDynamicSensor::maximumValue(sensor->sensorType(), sensor->length());
        QString inSuffix = KisDynamicSensor::valueSuffix(sensor->sensorType());

        int outMinValue = m_curveOption->intMinValue();
        int outMaxValue = m_curveOption->intMaxValue();
        QString outSuffix = m_curveOption->valueSuffix();

        m_curveOptionWidget->intIn->setSuffix(inSuffix);
        m_curveOptionWidget->intOut->setSuffix(outSuffix);

        m_curveOptionWidget->curveWidget->setupInOutControls(m_curveOptionWidget->intIn,m_curveOptionWidget->intOut,
                                                         inMinValue,inMaxValue,outMinValue,outMaxValue);
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

void KisCurveOptionWidget::updateLabelsOfCurrentSensor()
{
    updateSensorCurveLabels(m_curveOptionWidget->sensorSelector->currentHighlighted());
    updateCurve(m_curveOptionWidget->sensorSelector->currentHighlighted());
}

void KisCurveOptionWidget::updateValues()
{
    m_curveOption->setValue(m_curveOptionWidget->strengthSlider->value()/100.0); // convert back to 0-1 for data
    m_curveOption->setCurveUsed(m_curveOptionWidget->checkBoxUseCurve->isChecked());
    disableWidgets(!m_curveOptionWidget->checkBoxUseCurve->isChecked());
    emitSettingChanged();
}

void KisCurveOptionWidget::updateMode()
{
    m_curveOption->setCurveMode(m_curveOptionWidget->curveMode->currentIndex());
    emitSettingChanged();
}

void KisCurveOptionWidget::changeCurveLinear()
{
    QList<QPointF> points;
    points.push_back(QPointF(0,0));
    points.push_back(QPointF(1,1));
    m_curveOptionWidget->curveWidget->setCurve(KisCubicCurve(points));
}

void KisCurveOptionWidget::changeCurveReverseLinear()
{
        QList<QPointF> points;
        points.push_back(QPointF(0,1));
        points.push_back(QPointF(1,0));
        m_curveOptionWidget->curveWidget->setCurve(KisCubicCurve(points));
}

void KisCurveOptionWidget::changeCurveSShape()
{
        QList<QPointF> points;
        points.push_back(QPointF(0,0));
        points.push_back(QPointF(0.25,0.1));
        points.push_back(QPointF(0.75,0.9));
        points.push_back(QPointF(1, 1));
        m_curveOptionWidget->curveWidget->setCurve(KisCubicCurve(points));
}


void KisCurveOptionWidget::changeCurveReverseSShape()
{
        QList<QPointF> points;
        points.push_back(QPointF(0,1));
        points.push_back(QPointF(0.25,0.9));
        points.push_back(QPointF(0.75,0.1));
        points.push_back(QPointF(1,0));
        m_curveOptionWidget->curveWidget->setCurve(KisCubicCurve(points));
}

void KisCurveOptionWidget::changeCurveJShape()
{
    QList<QPointF> points;
    points.push_back(QPointF(0,0));
    points.push_back(QPointF(0.35,0.1));
    points.push_back(QPointF(1,1));
    m_curveOptionWidget->curveWidget->setCurve(KisCubicCurve(points));
}

void KisCurveOptionWidget::changeCurveLShape()
{
    QList<QPointF> points;
    points.push_back(QPointF(0,1));
    points.push_back(QPointF(0.25,0.48));
    points.push_back(QPointF(1,0));
    m_curveOptionWidget->curveWidget->setCurve(KisCubicCurve(points));
}

void KisCurveOptionWidget::changeCurveUShape()
{
    QList<QPointF> points;
    points.push_back(QPointF(0,1));
    points.push_back(QPointF(0.5,0));
    points.push_back(QPointF(1,1));
    m_curveOptionWidget->curveWidget->setCurve(KisCubicCurve(points));
}

void KisCurveOptionWidget::changeCurveArchShape()
{
    QList<QPointF> points;
    points.push_back(QPointF(0,0));
    points.push_back(QPointF(0.5,1));
    points.push_back(QPointF(1,0));
    m_curveOptionWidget->curveWidget->setCurve(KisCubicCurve(points));
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


void KisCurveOptionWidget::updateThemedIcons()
{
    // set all the icons for the curve preset shapes
    m_curveOptionWidget->linearCurveButton->setIcon(KisIconUtils::loadIcon("curve-preset-linear"));
    m_curveOptionWidget->revLinearButton->setIcon(KisIconUtils::loadIcon("curve-preset-linear-reverse"));
    m_curveOptionWidget->jCurveButton->setIcon(KisIconUtils::loadIcon("curve-preset-j"));
    m_curveOptionWidget->lCurveButton->setIcon(KisIconUtils::loadIcon("curve-preset-l"));
    m_curveOptionWidget->sCurveButton->setIcon(KisIconUtils::loadIcon("curve-preset-s"));
    m_curveOptionWidget->reverseSCurveButton->setIcon(KisIconUtils::loadIcon("curve-preset-s-reverse"));
    m_curveOptionWidget->uCurveButton->setIcon(KisIconUtils::loadIcon("curve-preset-u"));
    m_curveOptionWidget->revUCurveButton->setIcon(KisIconUtils::loadIcon("curve-preset-arch"));

    // this helps make the checkboxes show themselves on the dark color themes
    QPalette pal = m_curveOptionWidget->sensorSelector->palette();
    QPalette newPalette = pal;
    newPalette.setColor(QPalette::Active, QPalette::Background, pal.text().color() );
    m_curveOptionWidget->sensorSelector->setPalette(newPalette);

}

