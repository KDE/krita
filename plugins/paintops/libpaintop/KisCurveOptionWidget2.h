/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISCURVEOPTIONWIDGET2_H
#define KISCURVEOPTIONWIDGET2_H

#include <kis_paintop_option.h>

class Ui_WdgCurveOption2;
class KisCurveOption;
class QComboBox;

#include <kis_dynamic_sensor.h>
#include <KisCurveOptionData.h>
#include <lager/cursor.hpp>
#include <lager/constant.hpp>

class PAINTOP_EXPORT KisCurveOptionWidget2 : public KisPaintOpOption
{
    Q_OBJECT
public:
    KisCurveOptionWidget2(lager::cursor<KisCurveOptionData> optionData,
                          const QString &minLabel, const QString &maxLabel,
                          int minValue, int maxValue, const QString &valueSuffix,
                          lager::reader<bool> enabledLink = lager::make_constant(true));
    ~KisCurveOptionWidget2() override;

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;
    void lodLimitations(KisPaintopLodLimitations *l) const override;

    bool isCheckable() const override;
    bool isChecked() const override;
    void setChecked(bool checked) override;
    void show();
    virtual void setEnabled(bool enabled);

    void setCurveWidgetsEnabled(bool value);

protected:

    QWidget* curveWidget();

protected Q_SLOTS:
    void slotCurveChanged(const KisCubicCurve &curve);

    void updateSensorCurveLabels(const QString &sensorId, const int length);
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
    QWidget* m_widget {nullptr};
    Ui_WdgCurveOption2* m_curveOptionWidget {nullptr};
    QComboBox* m_curveMode {nullptr};
    struct Private;
    const QScopedPointer<Private> m_d;

    void hideRangeLabelsAndBoxes(bool isHidden);
};

#endif // KISCURVEOPTIONWIDGET2_H
