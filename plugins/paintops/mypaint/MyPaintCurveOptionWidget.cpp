/*
 * SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2009 Sven Langkamp <sven.langkamp@gmail.com>
 * SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 * SPDX-FileCopyrightText: 2020 Ashwin Dhakaita <ashwingpdhakaita@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "MyPaintCurveOptionWidget.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <kis_global.h>
#include <kis_icon_utils.h>
#include <kis_signals_blocker.h>
#include <widgets/kis_curve_widget.h>

#include "MyPaintBrushOption.h"
#include "MyPaintCurveOption.h"
#include "MyPaintPaintOpOption.h"
#include "MyPaintPaintOpSettingsWidget.h"
#include "ui_wdgmypaintcurveoption.h"

KisMyPaintCurveOptionWidget::KisMyPaintCurveOptionWidget(KisMyPaintCurveOption* curveOption, const QString &minLabel, const QString &maxLabel, bool hideSlider, KisMyPaintOpOption *baseOption)
    : KisCurveOptionWidget (curveOption, minLabel, maxLabel, hideSlider)
{
    Q_UNUSED(baseOption);
    setObjectName("KisMyPaintCurveOptionWidget");

    strengthToCurveOptionValueScale = 1.0;
    hideRangeLabelsAndBoxes(false);

    connect(m_curveOptionWidget->sensorSelector, SIGNAL(highlightedSensorChanged(KisDynamicSensorSP)), SLOT(updateRangeSpinBoxes(KisDynamicSensorSP)));
    connect(m_curveOptionWidget->xMinBox, SIGNAL(valueChanged(double)), SLOT(emitSettingChanged()));
    connect(m_curveOptionWidget->xMaxBox, SIGNAL(valueChanged(double)), SLOT(emitSettingChanged()));
    connect(m_curveOptionWidget->yMinBox, SIGNAL(valueChanged(double)), SLOT(emitSettingChanged()));
    connect(m_curveOptionWidget->yMaxBox, SIGNAL(valueChanged(double)), SLOT(emitSettingChanged()));

    m_curveOptionWidget->strengthSlider->setRange(curveOption->minValue(), curveOption->maxValue(), 2);
    m_curveOptionWidget->strengthSlider->setSingleStep(0.01);
    m_curveOptionWidget->strengthSlider->setValue(curveOption->value());
    m_curveOptionWidget->strengthSlider->setPrefix(i18n("Base Value: "));
    m_curveOptionWidget->strengthSlider->setSuffix("");

    if (hideSlider) {
        m_curveOptionWidget->strengthSlider->hide();
    }

    connect(m_curveOption, SIGNAL(unCheckUseCurve()), SLOT(slotUnCheckUseCurve()));
}

KisMyPaintCurveOptionWidget::~KisMyPaintCurveOptionWidget()
{
}

void KisMyPaintCurveOptionWidget::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    checkRanges();
    KisDynamicSensorSP dynamicSensor = m_curveOptionWidget->sensorSelector->currentHighlighted();
    KisMyPaintBrushOption* currentSensor = dynamic_cast<KisMyPaintBrushOption*>(m_curveOptionWidget->sensorSelector->currentHighlighted().data());
    KIS_SAFE_ASSERT_RECOVER(dynamicSensor && currentSensor) { }
    if (dynamicSensor) {
        setting->setProperty(m_curveOption->name() + dynamicSensor->identifier() + "XMIN", m_curveOptionWidget->xMinBox->value());
        setting->setProperty(m_curveOption->name() + dynamicSensor->identifier() + "XMAX", m_curveOptionWidget->xMaxBox->value());
        setting->setProperty(m_curveOption->name() + dynamicSensor->identifier() + "YMIN", m_curveOptionWidget->yMinBox->value());
        setting->setProperty(m_curveOption->name() + dynamicSensor->identifier() + "YMAX", m_curveOptionWidget->yMaxBox->value());
    }
    if (currentSensor) {

        currentSensor->setXRangeMin(m_curveOptionWidget->xMinBox->value());
        currentSensor->setXRangeMax(m_curveOptionWidget->xMaxBox->value());
        currentSensor->setYRangeMin(m_curveOptionWidget->yMinBox->value());
        currentSensor->setYRangeMax(m_curveOptionWidget->yMaxBox->value());
    }
    if (dynamicSensor) {
        // don't use currentSensor here, because it would get converted to shared pointer
        updateSensorCurveLabels(dynamicSensor);
    }
    setBaseValue(setting, m_curveOptionWidget->strengthSlider->value());
    m_curveOption->writeOptionSetting(setting);
}

void KisMyPaintCurveOptionWidget::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisCurveOptionWidget::readOptionSetting(setting);

    m_curveOptionWidget->checkBoxUseCurve->setChecked(m_curveOption->isCurveUsed());
    m_curveOptionWidget->strengthSlider->setValue(getBaseValue(setting));
    updateRangeSpinBoxes(m_curveOptionWidget->sensorSelector->currentHighlighted());
}

void KisMyPaintCurveOptionWidget::slotUnCheckUseCurve() {

    m_curveOptionWidget->checkBoxUseCurve->setChecked(false);
    updateValues();
}

void KisMyPaintCurveOptionWidget::updateSensorCurveLabels(KisDynamicSensorSP sensor) const
{
    KisCurveOptionWidget::updateSensorCurveLabels(sensor);
    KisMyPaintBrushOption* mySensor = dynamic_cast<KisMyPaintBrushOption*>(sensor.data());
    if (mySensor) {

        m_curveOptionWidget->label_xmin->setText(mySensor->minimumXLabel());
        m_curveOptionWidget->label_xmax->setText(mySensor->maximumXLabel());
        m_curveOptionWidget->label_ymin->setText(mySensor->minimumYLabel());
        m_curveOptionWidget->label_ymax->setText(mySensor->maximumYLabel());
    }
}

void KisMyPaintCurveOptionWidget::updateRangeSpinBoxes(KisDynamicSensorSP sensor) const {

    KisMyPaintBrushOption* mySensor = dynamic_cast<KisMyPaintBrushOption*>(sensor.data());
    m_curveOptionWidget->xMinBox->blockSignals(true);
    m_curveOptionWidget->xMaxBox->blockSignals(true);
    m_curveOptionWidget->yMinBox->blockSignals(true);
    m_curveOptionWidget->yMaxBox->blockSignals(true);

    m_curveOptionWidget->xMinBox->setValue( mySensor->getXRangeMin());
    m_curveOptionWidget->xMaxBox->setValue( mySensor->getXRangeMax());
    m_curveOptionWidget->yMinBox->setValue( mySensor->getYRangeMin());
    m_curveOptionWidget->yMaxBox->setValue( mySensor->getYRangeMax());

    m_curveOptionWidget->xMinBox->blockSignals(false);
    m_curveOptionWidget->xMaxBox->blockSignals(false);
    m_curveOptionWidget->yMinBox->blockSignals(false);
    m_curveOptionWidget->yMaxBox->blockSignals(false);
}

float KisMyPaintCurveOptionWidget::getBaseValue(KisPropertiesConfigurationSP setting) {

    KisMyPaintCurveOption *curveOpt = dynamic_cast<KisMyPaintCurveOption*>(m_curveOption);
    if(curveOpt->currentSetting() == MYPAINT_BRUSH_SETTING_RADIUS_LOGARITHMIC)
        return log(setting->getFloat(MYPAINT_DIAMETER)/2);

    if(curveOpt->currentSetting() == MYPAINT_BRUSH_SETTING_OPAQUE)
        return setting->getFloat(MYPAINT_OPACITY);

    if(curveOpt->currentSetting() == MYPAINT_BRUSH_SETTING_HARDNESS)
        return setting->getFloat(MYPAINT_HARDNESS);

    MyPaintBrush *brush = mypaint_brush_new();
    mypaint_brush_from_string(brush, setting->getProperty(MYPAINT_JSON).toByteArray());

    float ret = mypaint_brush_get_base_value(brush, curveOpt->currentSetting());

    mypaint_brush_unref(brush);
    return ret;
}

void KisMyPaintCurveOptionWidget::setBaseValue(KisPropertiesConfigurationSP setting, float val) const {

    QJsonDocument doc = QJsonDocument::fromJson(setting->getProperty(MYPAINT_JSON).toByteArray());
    QJsonObject brush_json = doc.object();
    QVariantMap map = brush_json.toVariantMap();
    QVariantMap settings_map = map["settings"].toMap();
    QVariantMap name_map = settings_map[m_curveOption->name()].toMap();
    double base_val = name_map["base_value"].toDouble();
    Q_UNUSED(base_val);

    name_map["base_value"] = val;
    settings_map[m_curveOption->name()] = name_map;
    map["settings"] = settings_map;

    QJsonObject json_obj = QJsonObject::fromVariantMap(map);
    QJsonDocument doc2(json_obj);

    setting->setProperty(MYPAINT_JSON, doc2.toJson());

    KisMyPaintCurveOption *curveOpt = dynamic_cast<KisMyPaintCurveOption*>(m_curveOption);
    if(curveOpt->currentSetting() == MYPAINT_BRUSH_SETTING_RADIUS_LOGARITHMIC)
        setting->setProperty(MYPAINT_DIAMETER, exp(val)*2);

    if(curveOpt->currentSetting() == MYPAINT_BRUSH_SETTING_HARDNESS)
        setting->setProperty(MYPAINT_HARDNESS, val);

    if(curveOpt->currentSetting() == MYPAINT_BRUSH_SETTING_OPAQUE)
        setting->setProperty(MYPAINT_OPACITY, val);

    if(curveOpt->currentSetting() == MYPAINT_BRUSH_SETTING_OFFSET_BY_RANDOM)
        setting->setProperty(MYPAINT_OFFSET_BY_RANDOM, val);
}

void KisMyPaintCurveOptionWidget::checkRanges() const
{
    if(m_curveOptionWidget->xMinBox->value() >= m_curveOptionWidget->xMaxBox->value()) {

        m_curveOptionWidget->xMinBox->blockSignals(true);
        m_curveOptionWidget->xMinBox->setValue(m_curveOptionWidget->xMaxBox->value() - 1);
        m_curveOptionWidget->xMinBox->blockSignals(false);
    }

    if(m_curveOptionWidget->yMinBox->value() >= m_curveOptionWidget->yMaxBox->value()) {

        m_curveOptionWidget->yMinBox->blockSignals(true);
        m_curveOptionWidget->yMinBox->setValue(m_curveOptionWidget->yMaxBox->value() - 1);
        m_curveOptionWidget->yMinBox->blockSignals(false);
    }
}

KisDoubleSliderSpinBox* KisMyPaintCurveOptionWidget::slider() {

    return m_curveOptionWidget->strengthSlider;
}

void KisMyPaintCurveOptionWidget::refresh() {

    emitSettingChanged();
}
