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
#include <KisCurveRangeModelInterface.h>
#include <KisCurveOptionInputControlsStrategyInterface.h>
#include <lager/cursor.hpp>
#include <lager/constant.hpp>


class PAINTOP_EXPORT KisCurveOptionWidget2 : public KisPaintOpOption
{
    Q_OBJECT
public:
    using data_type = KisCurveOptionDataCommon;

    KisCurveOptionWidget2(lager::cursor<KisCurveOptionDataCommon> optionData,
                          KisPaintOpOption::PaintopCategory category,
                          lager::reader<bool> enabledLink = lager::make_constant(true),
                          std::optional<lager::reader<std::tuple<qreal, qreal>>> rangeReader = std::nullopt,
                          KisCurveRangeModelFactory curveRangeFactory = {},
                          KisCurveOptionInputControlsStrategyFactory inputControlsFactory = {});

    KisCurveOptionWidget2(lager::cursor<KisCurveOptionDataCommon> optionData,
                          KisPaintOpOption::PaintopCategory category,
                          const QString &curveMinLabel, const QString &curveMaxLabel,
                          lager::reader<bool> enabledLink = lager::make_constant(true),
                          std::optional<lager::reader<std::tuple<qreal, qreal>>> rangeReader = std::nullopt,
                          KisCurveRangeModelFactory curveRangeFactory = {},
                          KisCurveOptionInputControlsStrategyFactory inputControlsFactory = {});

    KisCurveOptionWidget2(lager::cursor<KisCurveOptionDataCommon> optionData,
                          KisPaintOpOption::PaintopCategory category,
                          const QString &curveMinLabel, const QString &curveMaxLabel,
                          int curveMinValue, int curveMaxValue, const QString &curveValueSuffix,
                          lager::reader<bool> enabledLink = lager::make_constant(true),
                          std::optional<lager::reader<std::tuple<qreal, qreal>>> rangeReader = std::nullopt,
                          KisCurveRangeModelFactory curveRangeFactory = {},
                          KisCurveOptionInputControlsStrategyFactory inputControlsFactory = {});
private:
    KisCurveOptionWidget2(lager::cursor<KisCurveOptionDataCommon> optionData,
                          KisPaintOpOption::PaintopCategory category,
                          const QString &curveMinLabel, const QString &curveMaxLabel,
                          int curveMinValue, int curveMaxValue, const QString &curveValueSuffix,
                          const QString &strengthPrefix, const QString &strengthSuffix,
                          lager::reader<bool> enabledLink = lager::make_constant(true),
                          std::optional<lager::reader<std::tuple<qreal, qreal>>> rangeReader = std::nullopt,
                          KisCurveRangeModelFactory curveRangeFactory = {},
                          KisCurveOptionInputControlsStrategyFactory inputControlsFactory = {});

public:
    ~KisCurveOptionWidget2() override;

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

    bool isCheckable() const override;
    void show();

protected:

    void setCurveWidgetsEnabled(bool value);
    QWidget* curveWidget();

protected Q_SLOTS:
    void slotCurveChanged(const KisCubicCurve &curve);

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
    QScopedPointer<KisCurveOptionInputControlsStrategyInterface> m_curveInputControlsStrategy;
    QComboBox* m_curveMode {nullptr};
    struct Private;
    const QScopedPointer<Private> m_d;
    void hideRangeLabelsAndBoxes(bool isHidden);
};

#endif // KISCURVEOPTIONWIDGET2_H
