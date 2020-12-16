/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2009 Sven Langkamp <sven.langkamp@gmail.com>
 * SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KIS_CURVE_OPTION_WIDGET_H
#define KIS_CURVE_OPTION_WIDGET_H

#include <kis_paintop_option.h>

class Ui_WdgCurveOption;
class KisCurveOption;
class QComboBox;

#include <kis_dynamic_sensor.h>

class PAINTOP_EXPORT KisCurveOptionWidget : public KisPaintOpOption
{
    Q_OBJECT
public:
    KisCurveOptionWidget(KisCurveOption* curveOption, const QString &minLabel, const QString &maxLabel, bool hideSlider = false);
    ~KisCurveOptionWidget() override;

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;
    void lodLimitations(KisPaintopLodLimitations *l) const override;

    bool isCheckable() const override;
    bool isChecked() const override;
    void setChecked(bool checked) override;
    void show();
    virtual void setEnabled(bool enabled);

protected:

    KisCurveOption* curveOption();
    QWidget* curveWidget();

protected Q_SLOTS:

    void slotModified();
    void slotUseSameCurveChanged();

    virtual void updateSensorCurveLabels(KisDynamicSensorSP sensor) const;
    void updateCurve(KisDynamicSensorSP sensor);
    virtual void updateValues();
    void updateMode();
    void updateLabelsOfCurrentSensor();
    void disableWidgets(bool disable);
    void updateThemedIcons();


    // curve shape preset buttons
    void changeCurveLinear();
    void changeCurveReverseLinear();
    void changeCurveSShape();
    void changeCurveReverseSShape();
    void changeCurveJShape();
    void changeCurveLShape();
    void changeCurveUShape();
    void changeCurveArchShape();


protected:
    QWidget* m_widget;
    Ui_WdgCurveOption* m_curveOptionWidget;
    QComboBox* m_curveMode;
    KisCurveOption* m_curveOption;
    qreal strengthToCurveOptionValueScale;

    void hideRangeLabelsAndBoxes(bool isHidden);
    KisCubicCurve getWidgetCurve();
    KisCubicCurve getHighlightedSensorCurve();

};

#endif // KIS_CURVE_OPTION_WIDGET_H
