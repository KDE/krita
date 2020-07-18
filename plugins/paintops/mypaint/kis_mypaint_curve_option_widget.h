#ifndef KIS_MYPAINT_CURVE_OPTION_WIDGET_H
#define KIS_MYPAINT_CURVE_OPTION_WIDGET_H

#include <QObject>
#include <kis_paintop_option.h>
#include "ui_wdgmypaintcurveoption.h"
#include <kis_mypaint_curve_option.h>
#include <kis_my_paintop_option.h>

class Ui_WdgMyPaintCurveOption;
class KisMyPaintCurveOption;
class QComboBox;

class PAINTOP_EXPORT KisMyPaintCurveOptionWidget : public KisPaintOpOption
{
    Q_OBJECT
public:
    KisMyPaintCurveOptionWidget(KisMyPaintCurveOption* curveOption, const QString &minLabel, const QString &maxLabel, bool hideSlider = false, KisMyPaintOpOption *option = nullptr);
    ~KisMyPaintCurveOptionWidget() override;

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;
    void lodLimitations(KisPaintopLodLimitations *l) const override;

    bool isCheckable() const override;
    bool isChecked() const override;
    void setChecked(bool checked) override;
    void show();

    KisDoubleSliderSpinBox* slider();

protected:

    KisMyPaintCurveOption* curveOption();
    QWidget* curveWidget();

private Q_SLOTS:

    void slotModified();
    void slotUseSameCurveChanged();
    void slotCheckUseCurve();

    void updateSensorCurveLabels(KisDynamicOptionSP sensor);
    void updateCurve(KisDynamicOptionSP sensor);
    void updateValues();
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


private:
    QWidget* m_widget;
    Ui_WdgMyPaintCurveOption* m_curveOptionWidget;
    QComboBox* m_curveMode;
    KisMyPaintCurveOption* m_curveOption;
    KisMyPaintOpOption *m_baseOption;
    bool updateValuesCalled = false;

    KisCubicCurve getWidgetCurve();
    KisCubicCurve getHighlightedSensorCurve();
    float getBaseValue(KisPropertiesConfigurationSP setting);
    void setBaseValue(KisPropertiesConfigurationSP setting, float val) const;

};

#endif // KIS_MYPAINT_CURVE_OPTION_WIDGET_H
