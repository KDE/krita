#ifndef KIS_MYPAINT_CURVE_OPTION_WIDGET_H
#define KIS_MYPAINT_CURVE_OPTION_WIDGET_H

#include <QObject>
#include <kis_paintop_option.h>
#include <kis_curve_option_widget.h>
#include "ui_wdgmypaintcurveoption.h"
#include "ui_wdgcurveoption.h"
#include <kis_mypaint_curve_option.h>
#include <kis_my_paintop_option.h>

class Ui_WdgMyPaintCurveOption;
class KisMyPaintCurveOption;
class QComboBox;

class PAINTOP_EXPORT KisMyPaintCurveOptionWidget : public KisCurveOptionWidget
{
    Q_OBJECT
public:
    KisMyPaintCurveOptionWidget(KisMyPaintCurveOption* curveOption, const QString &minLabel, const QString &maxLabel, bool hideSlider = false, KisMyPaintOpOption *option = nullptr);
    ~KisMyPaintCurveOptionWidget() override;

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

    KisDoubleSliderSpinBox* slider();

protected Q_SLOTS:

    void slotUnCheckUseCurve();

    void updateSensorCurveLabels(KisDynamicSensorSP sensor) const override;
    void updateRangeSpinBoxes(KisDynamicSensorSP sensor) const;

public Q_SLOTS:
    void refresh();

protected:

    KisMyPaintOpOption *m_baseOption;

    void checkRanges() const;
    float getBaseValue(KisPropertiesConfigurationSP setting);
    void setBaseValue(KisPropertiesConfigurationSP setting, float val) const;

};

#endif // KIS_MYPAINT_CURVE_OPTION_WIDGET_H
