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
#include <MyPaintStandardOptionData.h>

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
                     BASIC);
    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintRadiusByRandomData()),
                     BASIC);
    addPaintOpOption(hardnessWidget,
                     BASIC);
    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintAntiAliasingData()),
                     BASIC);
    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintEllipticalDabAngleData(), "°"),
                     BASIC);
    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintEllipticalDabRatioData()),
                     BASIC);
    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintDirectionFilterData()),
                     BASIC);
    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintSnapToPixelsData()),
                     BASIC);
    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintPressureGainData()),
                     BASIC);

    addPaintOpOption(kpowu::createOptionWidget<KisAirbrushOptionWidget>(),
                     AIRBRUSH);

    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintChangeColorHData()),
                     COLOR);
    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintChangeColorLData()),
                     COLOR);
    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintChangeColorVData()),
                     COLOR);
    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintChangeColorHSLSData()),
                     COLOR);
    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintChangeColorHSVSData()),
                     COLOR);
    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintColorizeData()),
                     COLOR);

    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintFineSpeedGammaData()),
                     SPEED);
    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintGrossSpeedGammaData()),
                     SPEED);
    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintFineSpeedSlownessData()),
                     SPEED);
    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintGrossSpeedSlownessData()),
                     SPEED);
    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintOffsetBySpeedData()),
                     SPEED);
    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintOffsetBySpeedFilterData()),
                     SPEED);
    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintOffsetByRandomData()),
                     SPEED);

    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintDabsPerBasicRadiusData()),
                     DABS);
    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintDabsPerActualRadiusData()),
                     DABS);
    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintDabsPerSecondData()),
                     DABS);

    addPaintOpOption(opacityWidget,
                     OPACITY);
    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintOpaqueLinearizeData()),
                     OPACITY);
    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintOpaqueMultiplyData()),
                     OPACITY);

    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintSlowTrackingPerDabData()),
                     TRACKING);
    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintSlowTrackingData()),
                     TRACKING);
    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintTrackingNoiseData()),
                     TRACKING);

    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintSmudgeData()),
                     SMUDGE);
    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintSmudgeLengthData()),
                     SMUDGE);
    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintSmudgeRadiusLogData()),
                     SMUDGE);

    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintStrokeDurationLogData()),
                     STROKE);
    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintStrokeHoldtimeData()),
                     STROKE);
    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintStrokeThresholdData()),
                     STROKE);

    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintCustomInputData()),
                     CUSTOM);
    addPaintOpOption(kpowu::createMyPaintCurveOptionWidget(MyPaintCustomInputSlownessData()),
                     CUSTOM);
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

void KisMyPaintOpSettingsWidget::addPaintOpOption(KisPaintOpOption *option, MyPaintPaintopCategory id)
{
    QString category;

    switch (id) {
    case BASIC:
        category = i18nc("Option Category", "Basic");
        break;
    case AIRBRUSH:
        category = i18n("Airbrush");
        break;
    case COLOR:
        category = i18nc("Option Category", "Color");
        break;
    case SPEED:
        category = i18nc("Option Category", "Speed");
        break;
    case DABS:
        category = i18nc("Option Category", "Dabs");
        break;
    case OPACITY:
        category = i18nc("Option Category", "Opacity");
        break;
    case TRACKING:
        category = i18nc("Option Category", "Tracking");
        break;
    case SMUDGE:
        category = i18nc("Option Category", "Smudge");
        break;
    case STROKE:
        category = i18nc("Option Category", "Stroke");
        break;
    case CUSTOM:
        category = i18nc("Option Category", "Custom");
        break;
    }

    return KisPaintOpSettingsWidget::addPaintOpOption(option, category);
}
