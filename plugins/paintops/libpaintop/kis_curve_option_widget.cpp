/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2009 Sven Langkamp <sven.langkamp@gmail.com>
 * SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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

    strengthToCurveOptionValueScale = 100.0;
    hideRangeLabelsAndBoxes(true);

    m_curveOptionWidget->sensorSelector->setCurveOption(curveOption);

    updateSensorCurveLabels(m_curveOptionWidget->sensorSelector->currentHighlighted());
    updateCurve(m_curveOptionWidget->sensorSelector->currentHighlighted());

    connect(m_curveOptionWidget->curveWidget, SIGNAL(modified()), this, SLOT(slotModified()));
    connect(m_curveOptionWidget->sensorSelector, SIGNAL(parametersChanged()), SLOT(emitSettingChanged()));
    connect(m_curveOptionWidget->sensorSelector, SIGNAL(parametersChanged()), SLOT(updateLabelsOfCurrentSensor()));
    connect(m_curveOptionWidget->sensorSelector, SIGNAL(highlightedSensorChanged(KisDynamicSensorSP)), SLOT(updateSensorCurveLabels(KisDynamicSensorSP)));
    connect(m_curveOptionWidget->sensorSelector, SIGNAL(highlightedSensorChanged(KisDynamicSensorSP)), SLOT(updateCurve(KisDynamicSensorSP)));
    connect(m_curveOptionWidget->checkBoxUseSameCurve, SIGNAL(stateChanged(int)), SLOT(slotUseSameCurveChanged()));


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

    {
        // Signals needs to be blocked, otherwise checking the checkbox will trigger
        //   setting the common curve to the widget curve, which is incorrect in this case.
        KisSignalsBlocker b(m_curveOptionWidget->checkBoxUseSameCurve,
                            m_curveOptionWidget->checkBoxUseCurve,
                            m_curveOptionWidget->strengthSlider,
                            m_curveOptionWidget->curveMode);


        m_curveOptionWidget->checkBoxUseSameCurve->setChecked(m_curveOption->isSameCurveUsed());
        m_curveOptionWidget->checkBoxUseCurve->setChecked(m_curveOption->isCurveUsed());
        m_curveOptionWidget->strengthSlider->setValue(m_curveOption->value()*100);
        m_curveOptionWidget->curveMode->setCurrentIndex(m_curveOption->getCurveMode());
    }

    disableWidgets(!m_curveOption->isCurveUsed());

    m_curveOptionWidget->sensorSelector->reload();
    if(m_curveOption->activeSensors().size() > 0)
        m_curveOptionWidget->sensorSelector->setCurrent(m_curveOption->activeSensors().first());
    updateSensorCurveLabels(m_curveOptionWidget->sensorSelector->currentHighlighted());
    updateCurve(m_curveOptionWidget->sensorSelector->currentHighlighted());
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

void KisCurveOptionWidget::setEnabled(bool enabled)
{
    m_widget->setEnabled(enabled);
}

void KisCurveOptionWidget::updateRange(qreal minValue, qreal maxValue)
{
    m_curveOption->updateRange(minValue, maxValue);

    // strength settings is shown as 0-100%
    m_curveOptionWidget->strengthSlider->setRange(m_curveOption->minValue()*100, m_curveOption->maxValue()*100, 0);
    m_curveOptionWidget->strengthSlider->setValue(m_curveOption->value()*100);
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
    KisCubicCurve currentCurve = getWidgetCurve();
    if (m_curveOption->isSameCurveUsed()) {
        m_curveOption->setCommonCurve(currentCurve);
    } else {
        m_curveOptionWidget->sensorSelector->currentHighlighted()->setCurve(currentCurve);
    }
    emitSettingChanged();
}

void KisCurveOptionWidget::slotUseSameCurveChanged()
{
    // this is a slot that answers on "Share Curve across all settings" checkbox
    m_curveOption->setUseSameCurve(m_curveOptionWidget->checkBoxUseSameCurve->isChecked());
    KisCubicCurve currentCurve = getWidgetCurve();
    if (m_curveOption->isSameCurveUsed()) {
        m_curveOption->setCommonCurve(currentCurve);
    } else {
        for (auto sensor : m_curveOption->activeSensors()) {
            sensor->setCurve(currentCurve);
        }
        // set the current curve since it might be disabled
        m_curveOptionWidget->sensorSelector->currentHighlighted()->setCurve(currentCurve);
    }
    emitSettingChanged();
}

void KisCurveOptionWidget::updateSensorCurveLabels(KisDynamicSensorSP sensor) const
{
    if (sensor) {
        m_curveOptionWidget->label_xmin->setText(sensor->minimumLabel(sensor->sensorType()));
        m_curveOptionWidget->label_xmax->setText(sensor->maximumLabel(sensor->sensorType(), sensor->length()));

        int inMinValue = sensor->minimumValue(sensor->sensorType());
        int inMaxValue = sensor->maximumValue(sensor->sensorType(), sensor->length());
        QString inSuffix = sensor->valueSuffix(sensor->sensorType());

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
        KisCubicCurve curve = m_curveOption->isSameCurveUsed() ? m_curveOption->getCommonCurve() : sensor->curve();
        m_curveOptionWidget->curveWidget->setCurve(curve);
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
    m_curveOption->setValue(m_curveOptionWidget->strengthSlider->value()/strengthToCurveOptionValueScale); // convert back to 0-1 for data
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

void KisCurveOptionWidget::hideRangeLabelsAndBoxes(bool isHidden) {

    m_curveOptionWidget->xMaxBox->setHidden(isHidden);
    m_curveOptionWidget->xMinBox->setHidden(isHidden);
    m_curveOptionWidget->yMaxBox->setHidden(isHidden);
    m_curveOptionWidget->yMinBox->setHidden(isHidden);

    m_curveOptionWidget->xRangeLabel->setHidden(isHidden);
    m_curveOptionWidget->yRangeLabel->setHidden(isHidden);
    m_curveOptionWidget->toLabel1->setHidden(isHidden);
    m_curveOptionWidget->toLabel2->setHidden(isHidden);

}

KisCubicCurve KisCurveOptionWidget::getWidgetCurve()
{
    return m_curveOptionWidget->curveWidget->curve();
}

KisCubicCurve KisCurveOptionWidget::getHighlightedSensorCurve()
{
    return m_curveOptionWidget->sensorSelector->currentHighlighted()->curve();
}
