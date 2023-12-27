/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisDynamicSensorFactoryDrawingAngle.h"

#include <QCheckBox>
#include <QLabel>
#include <QHBoxLayout>
#include <kis_slider_spin_box.h>

#include "KisDrawingAngleSensorModel.h"
#include "KisWidgetConnectionUtils.h"
#include "KisKritaSensorPack.h"

namespace {
    auto safeDereferenceDrawingAngleSensor = lager::lenses::getset(
    [](const KisCurveOptionDataCommon &data) -> KisDrawingAngleSensorData {
        const KisKritaSensorPack *pack = dynamic_cast<const KisKritaSensorPack*>(data.sensorData.constData());
        if (pack) {
            return pack->constSensorsStruct().sensorDrawingAngle;
        } else {
            qWarning() << "safeDereferenceDrawingAngleSensor(get): failed to get a Krita sensor data";
            return KisDrawingAngleSensorData();
        }
    },
    [](KisCurveOptionDataCommon data, KisDrawingAngleSensorData sensor) -> KisCurveOptionDataCommon {
        KisKritaSensorPack *pack = dynamic_cast<KisKritaSensorPack*>(data.sensorData.data());
        if (pack) {
            pack->sensorsStruct().sensorDrawingAngle = sensor;
        } else {
            qWarning() << "safeDereferenceDrawingAngleSensor(set): failed to get a Krita sensor data";
        }
        return data;
    });
}

KisDynamicSensorFactoryDrawingAngle::KisDynamicSensorFactoryDrawingAngle()
    : KisSimpleDynamicSensorFactory(DrawingAngleId.id(), 0, 360, i18n("0°"), i18n("360°"), i18n("°"))
{

}

QWidget *KisDynamicSensorFactoryDrawingAngle::createConfigWidget(lager::cursor<KisCurveOptionDataCommon> data, QWidget *parent)
{
    QWidget *widget = new QWidget(parent);

    KisDrawingAngleSensorModel *model =
        new KisDrawingAngleSensorModel(data.zoom(safeDereferenceDrawingAngleSensor), widget);

    using namespace KisWidgetConnectionUtils;

    QCheckBox *chkLockedMode = new QCheckBox(i18nc("Lock Drawing angle at start of each stroke", "Lock"), widget);
    connectControl(chkLockedMode, model, "lockedAngleMode");

    QCheckBox *chkFanCorners = new QCheckBox(i18nc("Smoothing Drawing angle when a stroke is making a sharp turn", "Fan Corners"), widget);
    connectControl(chkFanCorners, model, "fanCornersEnabled");

    KisSliderSpinBox *intFanCornersStep = new KisSliderSpinBox(widget);
    intFanCornersStep->setRange(5, 90);
    intFanCornersStep->setSingleStep(1);
    intFanCornersStep->setSuffix(i18n("°"));
    connectControl(intFanCornersStep, model, "fanCornersStep");

    KisSliderSpinBox *angleOffset = new KisSliderSpinBox(widget);
    angleOffset->setRange(0, 359);
    angleOffset->setSingleStep(1);
    angleOffset->setSuffix(i18n("°"));
    connectControl(angleOffset, model, "angleOffset");

    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->addWidget(chkLockedMode);
    layout->addWidget(chkFanCorners);
    layout->addWidget(intFanCornersStep);
    layout->addWidget(new QLabel(i18n("Angle Offset")));
    layout->addWidget(angleOffset);

    return widget;
}

