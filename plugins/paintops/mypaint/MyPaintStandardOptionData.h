/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef MYPAINTSTANDARDOPTIONDATA_H
#define MYPAINTSTANDARDOPTIONDATA_H

#include <MyPaintCurveOptionData.h>
#include <kis_paintop_lod_limitations.h>


struct MyPaintRadiusLogarithmicData : MyPaintCurveOptionData
{
    MyPaintRadiusLogarithmicData();
};

struct MyPaintHardnessData : MyPaintCurveOptionData
{
    MyPaintHardnessData();
};

struct MyPaintOpacityData : MyPaintCurveOptionData
{
    MyPaintOpacityData();
};

struct MyPaintRadiusByRandomData : MyPaintCurveOptionData
{
    MyPaintRadiusByRandomData()
        : MyPaintCurveOptionData(KoID("radius_by_random", i18n("Radius by Random")),
                                 false, true, 0.0, 1.50)
    {
    }

    KisPaintopLodLimitations lodLimitations() const {
        KisPaintopLodLimitations l;
        if (qAbs(strengthValue) > 0.05) {
            l.limitations << KoID("Radius by Random", i18nc("PaintOp instant preview limitation", "Radius by Random, consider disabling Instant Preview"));
        }
        return l;
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

struct MyPaintPosterizeData : MyPaintCurveOptionData
{
    MyPaintPosterizeData()
        : MyPaintCurveOptionData(KoID("posterize", i18n("Posterize")),
                                 false, true, 0.0, 1.0)
    {
    }
};

struct MyPaintPosterizationLevelsData : MyPaintCurveOptionData
{
    MyPaintPosterizationLevelsData()
        : MyPaintCurveOptionData(KoID("posterize_num", i18n("Posterization Levels")),
                                 false, true, 0.0, 1.28)
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

    KisPaintopLodLimitations lodLimitations() const {
        KisPaintopLodLimitations l;
        if (qAbs(strengthValue) > 0.05) {
            l.limitations << KoID("Offset by Random", i18nc("PaintOp instant preview limitation", "Offset by Random, consider disabling Instant Preview"));
        }
        return l;
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

struct MyPaintSmudgeLengthMultiplierData : MyPaintCurveOptionData
{
    MyPaintSmudgeLengthMultiplierData()
        : MyPaintCurveOptionData(KoID("smudge_length_log", i18n("Smudge Length Multiplier")),
                                 false, true, 0.0, 20)
    {
    }
};

struct MyPaintSmudgeBucketData : MyPaintCurveOptionData
{
    MyPaintSmudgeBucketData()
        : MyPaintCurveOptionData(KoID("smudge_bucket", i18n("Smudge Bucket")),
                                 false, true, 0.0, 255.0)
    {
    }
};

struct MyPaintSmudgeTransparencyData : MyPaintCurveOptionData
{
    MyPaintSmudgeTransparencyData()
        : MyPaintCurveOptionData(KoID("smudge_transparency", i18n("Smudge Transparency")),
                                 false, true, -1.0, 1.0)
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

#endif // MYPAINTSTANDARDOPTIONDATA_H
