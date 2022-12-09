/*
 * SPDX-FileCopyrightText: 2020 Ashwin Dhakaita <ashwingpdhakaita@gmail.com>
 * SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "MyPaintPaintOpSettingsWidget.h"

#include <klocalizedstring.h>

#include "MyPaintPaintOpSettings.h"
#include <KisAirbrushOptionWidget.h>
#include <KisPaintOpOptionWidgetUtils.h>

#include <MyPaintCurveOptionData.h>
#include <MyPaintCurveOptionWidget2.h>
#include <MyPaintBasicOptionWidget.h>

struct MyPaintRadiusLogarithmicData : MyPaintCurveOptionData
{
    MyPaintRadiusLogarithmicData()
        : MyPaintCurveOptionData(KoID("radius_logarithmic",
                                        i18n("Radius Logarithmic")),
                                 false, true, 0.01, 8.0)
    {
    }
};

struct MyPaintHardnessData : MyPaintCurveOptionData
{
    MyPaintHardnessData()
        : MyPaintCurveOptionData(KoID("hardness", i18n("Hardness")),
                                 false, true, 0.02, 1.0)
    {
    }
};

struct MyPaintOpacityData : MyPaintCurveOptionData
{
    MyPaintOpacityData()
        : MyPaintCurveOptionData(KoID("opaque", i18n("Opaque")),
                                 false, true, 0.0, 1.0)
    {
    }
};

struct MyPaintRadiusByRandomData : MyPaintCurveOptionData
{
    MyPaintRadiusByRandomData()
        : MyPaintCurveOptionData(KoID("radius_by_random", i18n("Radius by Random")),
                                 false, true, 0.0, 1.50)
    {
    }
};

struct MyPaintAntiAliasingData : MyPaintCurveOptionData
{
    MyPaintAntiAliasingData()
        : MyPaintCurveOptionData(KoID("anti_aliasing", i18n("Anti Aliasing")),
                                 false, true, 0.0, 1.0)
    {
    }
};

struct MyPaintEllipticalDabAngleData : MyPaintCurveOptionData
{
    MyPaintEllipticalDabAngleData()
        : MyPaintCurveOptionData(KoID("elliptical_dab_angle",
                                      i18n("Elliptical Dab Angle")),
                                 false, true, 0.0, 180.0)
    {
    }
};

struct MyPaintEllipticalDabRatioData : MyPaintCurveOptionData
{
    MyPaintEllipticalDabRatioData()
        : MyPaintCurveOptionData(KoID("elliptical_dab_ratio", i18n("Elliptical Dab Ratio")),
                                 false, true, 1.0, 10.0)
    {
    }
};


struct MyPaintDirectionFilterData : MyPaintCurveOptionData
{
    MyPaintDirectionFilterData()
        : MyPaintCurveOptionData(KoID("direction_filter", i18n("Direction Filter")),
                                 false, true, 0.0, 10.0)
    {
    }
};

struct MyPaintSnapToPixelsData : MyPaintCurveOptionData
{
    MyPaintSnapToPixelsData()
        : MyPaintCurveOptionData(KoID("snap_to_pixel", i18n("Snap To Pixel")),
                                 false, true, 0.0, 10.0)
    {
    }
};


struct MyPaintPressureGainData : MyPaintCurveOptionData
{
    MyPaintPressureGainData()
        : MyPaintCurveOptionData(KoID("pressure_gain_log", i18n("Pressure Gain")),
                                 false, true, -1.8, 1.8)
    {
    }
};


struct MyPaintChangeColorHData : MyPaintCurveOptionData
{
    MyPaintChangeColorHData()
        : MyPaintCurveOptionData(KoID("change_color_h", i18n("Change Color H")),
                                 false, true, -2.0, 2.0)
    {
    }
};

struct MyPaintChangeColorLData : MyPaintCurveOptionData
{
    MyPaintChangeColorLData()
        : MyPaintCurveOptionData(KoID("change_color_l", i18n("Change Color L")),
                                 false, true, -2.0, 2.0)
    {
    }
};

struct MyPaintChangeColorVData : MyPaintCurveOptionData
{
    MyPaintChangeColorVData()
        : MyPaintCurveOptionData(KoID("change_color_v", i18n("Change Color V")),
                                 false, true, -2.0, 2.0)
    {
    }
};

struct MyPaintChangeColorHSLSData : MyPaintCurveOptionData
{
    MyPaintChangeColorHSLSData()
        : MyPaintCurveOptionData(KoID("change_color_hsl_s", i18n("Change Color HSL S")),
                                 false, true, -2.0, 2.0)
    {
    }
};

struct MyPaintChangeColorHSVSData : MyPaintCurveOptionData
{
    MyPaintChangeColorHSVSData()
        : MyPaintCurveOptionData(KoID("change_color_hsv_s", i18n("Change Color HSV S")),
                                 false, true, -2.0, 2.0)
    {
    }
};

struct MyPaintColorizeData : MyPaintCurveOptionData
{
    MyPaintColorizeData()
        : MyPaintCurveOptionData(KoID("colorize", i18n("Colorize")),
                                 false, true, 0.0, 1.0)
    {
    }
};

struct MyPaintFineSpeedGammaData : MyPaintCurveOptionData
{
    MyPaintFineSpeedGammaData()
        : MyPaintCurveOptionData(KoID("speed1_gamma", i18n("Fine Speed Gamma")),
                                 false, true, -8.0, 8.0)
    {
    }
};

struct MyPaintGrossSpeedGammaData : MyPaintCurveOptionData
{
    MyPaintGrossSpeedGammaData()
        : MyPaintCurveOptionData(KoID("speed2_gamma", i18n("Gross Speed Gamma")),
                                 false, true, -8.0, 8.0)
    {
    }
};

struct MyPaintFineSpeedSlownessData : MyPaintCurveOptionData
{
    MyPaintFineSpeedSlownessData()
        : MyPaintCurveOptionData(KoID("speed1_slowness", i18n("Fine Speed Slowness")),
                                 false, true, -8.0, 8.0)
    {
    }
};

struct MyPaintGrossSpeedSlownessData : MyPaintCurveOptionData
{
    MyPaintGrossSpeedSlownessData()
        : MyPaintCurveOptionData(KoID("speed2_slowness", i18n("Gross Speed Slowness")),
                                 false, true, -8.0, 8.0)
    {
    }
};

struct MyPaintOffsetBySpeedData : MyPaintCurveOptionData
{
    MyPaintOffsetBySpeedData()
        : MyPaintCurveOptionData(KoID("offset_by_speed", i18n("Offset By Speed")),
                                 false, true, -3.0, 3.0)
    {
    }
};

struct MyPaintOffsetBySpeedFilterData : MyPaintCurveOptionData
{
    MyPaintOffsetBySpeedFilterData()
        : MyPaintCurveOptionData(KoID("offset_by_speed_slowness", i18n("Offset by Speed Filter")),
                                 false, true, 0.0, 15.0)
    {
    }
};

struct MyPaintOffsetByRandomData : MyPaintCurveOptionData
{
    MyPaintOffsetByRandomData()
        : MyPaintCurveOptionData(KoID("offset_by_random", i18n("Offset By Random")),
                                 false, true, -3.0, 3.0)
    {
    }
};

struct MyPaintDabsPerBasicRadiusData : MyPaintCurveOptionData
{
    MyPaintDabsPerBasicRadiusData()
        : MyPaintCurveOptionData(KoID("dabs_per_basic_radius", i18n("Dabs Per Basic Radius")),
                                 false, true, 0.0, 6.0)
    {
    }
};

struct MyPaintDabsPerActualRadiusData : MyPaintCurveOptionData
{
    MyPaintDabsPerActualRadiusData()
        : MyPaintCurveOptionData(KoID("dabs_per_actual_radius", i18n("Dabs Per Actual Radius")),
                                 false, true, 0.0, 6.0)
    {
    }
};

struct MyPaintDabsPerSecondData : MyPaintCurveOptionData
{
    MyPaintDabsPerSecondData()
        : MyPaintCurveOptionData(KoID("dabs_per_second", i18n("Dabs per Second")),
                                 false, true, 0.0, 80.0)
    {
    }
};


struct MyPaintOpaqueLinearizeData : MyPaintCurveOptionData
{
    MyPaintOpaqueLinearizeData()
        : MyPaintCurveOptionData(KoID("opaque_linearize", i18n("Opaque Linearize")),
                                 false, true, 0.0, 3.0)
    {
    }
};

struct MyPaintOpaqueMultiplyData : MyPaintCurveOptionData
{
    MyPaintOpaqueMultiplyData()
        : MyPaintCurveOptionData(KoID("opaque_multiply", i18n("Opaque Multiply")),
                                 false, true, 0.0, 2.0)
    {
    }
};

struct MyPaintSlowTrackingPerDabData : MyPaintCurveOptionData
{
    MyPaintSlowTrackingPerDabData()
        : MyPaintCurveOptionData(KoID("slow_tracking_per_dab", i18n("Slow tracking per dab")),
                                 false, true, 0.0, 10.0)
    {
    }
};

struct MyPaintSlowTrackingData : MyPaintCurveOptionData
{
    MyPaintSlowTrackingData()
        : MyPaintCurveOptionData(KoID("slow_tracking", i18n("Slow Tracking")),
                                 false, true, 0.0, 10.0)
    {
    }
};

struct MyPaintTrackingNoiseData : MyPaintCurveOptionData
{
    MyPaintTrackingNoiseData()
        : MyPaintCurveOptionData(KoID("tracking_noise", i18n("Tracking Noise")),
                                 false, true, 0.0, 12.0)
    {
    }
};

struct MyPaintSmudgeData : MyPaintCurveOptionData
{
    MyPaintSmudgeData()
        : MyPaintCurveOptionData(KoID("smudge", i18n("Smudge")),
                                 false, true, 0.0, 1.0)
    {
    }
};

struct MyPaintSmudgeLengthData : MyPaintCurveOptionData
{
    MyPaintSmudgeLengthData()
        : MyPaintCurveOptionData(KoID("smudge_length", i18n("Smudge Length")),
                                 false, true, 0.0, 1.0)
    {
    }
};

struct MyPaintSmudgeRadiusLogData : MyPaintCurveOptionData
{
    MyPaintSmudgeRadiusLogData()
        : MyPaintCurveOptionData(KoID("smudge_radius_log", i18n("Smudge Radius Log")),
                                 false, true, -1.6, 1.6)
    {
    }
};

struct MyPaintStrokeDurationLogData : MyPaintCurveOptionData
{
    MyPaintStrokeDurationLogData()
        : MyPaintCurveOptionData(KoID("stroke_duration_logarithmic", i18n("Stroke Duration log")),
                                 false, true, -1.0, 7.0)
    {
    }
};

struct MyPaintStrokeHoldtimeData : MyPaintCurveOptionData
{
    MyPaintStrokeHoldtimeData()
        : MyPaintCurveOptionData(KoID("stroke_holdtime", i18n("Stroke Holdtime")),
                                 false, true, 0.0, 10.0)
    {
    }
};

struct MyPaintStrokeThresholdData : MyPaintCurveOptionData
{
    MyPaintStrokeThresholdData()
        : MyPaintCurveOptionData(KoID("stroke_threshold", i18n("Stroke Threshold")),
                                 false, true, 0.0, 0.5)
    {
    }
};

struct MyPaintCustomInputData : MyPaintCurveOptionData
{
    MyPaintCustomInputData()
        : MyPaintCurveOptionData(KoID("custom_input", i18n("Custom Input")),
                                 false, true, -5.0, 5.0)
    {
    }
};

struct MyPaintCustomInputSlownessData : MyPaintCurveOptionData
{
    MyPaintCustomInputSlownessData()
        : MyPaintCurveOptionData(KoID("custom_input_slowness", i18n("Custom Input Slowness")),
                                 false, true, 0.0, 10.0)
    {
    }
};

namespace KisPaintOpOptionWidgetUtils {

template <typename Data>
MyPaintCurveOptionWidget2* createMyPaintCurveOptionWidget(Data data, const QString &yValueSuffix = "")
{
    const qreal yLimit = qAbs(data.strengthMaxValue - data.strengthMinValue);
    return createOptionWidget<MyPaintCurveOptionWidget2>(std::move(data), yLimit, yValueSuffix);
}

} // namespace


KisMyPaintOpSettingsWidget:: KisMyPaintOpSettingsWidget(QWidget* parent)
    : KisPaintOpSettingsWidget(parent)
{
    /// TODO: move category into the widget itself, remove this
    /// overridden enum

    namespace kpowu = KisPaintOpOptionWidgetUtils;

    MyPaintCurveOptionWidget2 *radiusWidget =
        kpowu::createMyPaintCurveOptionWidget(MyPaintRadiusLogarithmicData());
    MyPaintCurveOptionWidget2 *hardnessWidget =
        kpowu::createMyPaintCurveOptionWidget(MyPaintHardnessData());
    MyPaintCurveOptionWidget2 *opacityWidget =
        kpowu::createMyPaintCurveOptionWidget(MyPaintOpacityData());

    KisPaintOpSettingsWidget::addPaintOpOption(
        kpowu::createOptionWidget<MyPaintBasicOptionWidget>(MyPaintBasicOptionData(),
                                                            radiusWidget->strengthValueDenorm(),
                                                            hardnessWidget->strengthValueDenorm(),
                                                            opacityWidget->strengthValueDenorm()));

    addPaintOpOption(radiusWidget,
                     KisMyPaintOpOption::BASIC);
    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintRadiusByRandomData()),
                     KisMyPaintOpOption::BASIC);
    addPaintOpOption(hardnessWidget,
                     KisMyPaintOpOption::BASIC);
    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintAntiAliasingData()),
                     KisMyPaintOpOption::BASIC);
    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintEllipticalDabAngleData(), "°"),
                     KisMyPaintOpOption::BASIC);
    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintEllipticalDabRatioData()),
                     KisMyPaintOpOption::BASIC);
    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintDirectionFilterData()),
                     KisMyPaintOpOption::BASIC);
    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintSnapToPixelsData()),
                     KisMyPaintOpOption::BASIC);
    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintPressureGainData()),
                     KisMyPaintOpOption::BASIC);

    addPaintOpOption(kpowu::createOptionWidget<KisAirbrushOptionWidget>(),
                     KisMyPaintOpOption::AIRBRUSH);

    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintChangeColorHData()),
                     KisMyPaintOpOption::COLOR);
    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintChangeColorLData()),
                     KisMyPaintOpOption::COLOR);
    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintChangeColorVData()),
                     KisMyPaintOpOption::COLOR);
    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintChangeColorHSLSData()),
                     KisMyPaintOpOption::COLOR);
    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintChangeColorHSVSData()),
                     KisMyPaintOpOption::COLOR);
    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintColorizeData()),
                     KisMyPaintOpOption::COLOR);

    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintFineSpeedGammaData()),
                     KisMyPaintOpOption::SPEED);
    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintGrossSpeedGammaData()),
                     KisMyPaintOpOption::SPEED);
    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintFineSpeedSlownessData()),
                     KisMyPaintOpOption::SPEED);
    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintGrossSpeedSlownessData()),
                     KisMyPaintOpOption::SPEED);
    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintOffsetBySpeedData()),
                     KisMyPaintOpOption::SPEED);
    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintOffsetBySpeedFilterData()),
                     KisMyPaintOpOption::SPEED);
    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintOffsetByRandomData()),
                     KisMyPaintOpOption::SPEED);

    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintDabsPerBasicRadiusData()),
                     KisMyPaintOpOption::DABS);
    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintDabsPerActualRadiusData()),
                     KisMyPaintOpOption::DABS);
    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintDabsPerSecondData()),
                     KisMyPaintOpOption::DABS);

    addPaintOpOption(opacityWidget,
                     KisMyPaintOpOption::OPACITY);
    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintOpaqueLinearizeData()),
                     KisMyPaintOpOption::OPACITY);
    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintOpaqueMultiplyData()),
                     KisMyPaintOpOption::OPACITY);

    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintSlowTrackingPerDabData()),
                     KisMyPaintOpOption::TRACKING);
    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintSlowTrackingData()),
                     KisMyPaintOpOption::TRACKING);
    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintTrackingNoiseData()),
                     KisMyPaintOpOption::TRACKING);

    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintSmudgeData()),
                     KisMyPaintOpOption::SMUDGE);
    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintSmudgeLengthData()),
                     KisMyPaintOpOption::SMUDGE);
    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintSmudgeRadiusLogData()),
                     KisMyPaintOpOption::SMUDGE);

    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintStrokeDurationLogData()),
                     KisMyPaintOpOption::STROKE);
    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintStrokeHoldtimeData()),
                     KisMyPaintOpOption::STROKE);
    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintStrokeThresholdData()),
                     KisMyPaintOpOption::STROKE);

    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintCustomInputData()),
                     KisMyPaintOpOption::CUSTOM);
    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintCustomInputSlownessData()),
                     KisMyPaintOpOption::CUSTOM);
}

KisMyPaintOpSettingsWidget::~ KisMyPaintOpSettingsWidget()
{
}

KisPropertiesConfigurationSP  KisMyPaintOpSettingsWidget::configuration() const
{
    KisMyPaintOpSettings* config = new KisMyPaintOpSettings(resourcesInterface());
    config->setProperty("paintop", "mypaintbrush"); // XXX: make this a const id string
    writeConfiguration(config);
    return config;
}

void KisMyPaintOpSettingsWidget::addPaintOpOption(KisPaintOpOption *option, KisMyPaintOpOption::PaintopCategory id)
{
    QString category;

    switch (id) {
    case KisMyPaintOpOption::BASIC:
        category = i18nc("Option Category", "Basic");
        break;
    case KisMyPaintOpOption::AIRBRUSH:
        category = i18n("Airbrush");
        break;
    case KisMyPaintOpOption::COLOR:
        category = i18nc("Option Category", "Color");
        break;
    case KisMyPaintOpOption::SPEED:
        category = i18nc("Option Category", "Speed");
        break;
    case KisMyPaintOpOption::DABS:
        category = i18nc("Option Category", "Dabs");
        break;
    case KisMyPaintOpOption::OPACITY:
        category = i18nc("Option Category", "Opacity");
        break;
    case KisMyPaintOpOption::TRACKING:
        category = i18nc("Option Category", "Tracking");
        break;
    case KisMyPaintOpOption::SMUDGE:
        category = i18nc("Option Category", "Smudge");
        break;
    case KisMyPaintOpOption::STROKE:
        category = i18nc("Option Category", "Stroke");
        break;
    case KisMyPaintOpOption::CUSTOM:
        category = i18nc("Option Category", "Custom");
        break;
    }

    return KisPaintOpSettingsWidget::addPaintOpOption(option, category);
}
