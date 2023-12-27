/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisDynamicSensorFactoryDistance.h"

#include "ui_SensorDistanceConfiguration.h"

#include <KisWidgetConnectionUtils.h>
#include <KisSensorWithLengthModel.h>
#include <KisKritaSensorPack.h>

namespace {
    auto safeDereferenceDistanceSensor = lager::lenses::getset(
    [](const KisCurveOptionDataCommon &data) -> KisSensorWithLengthData {
        const KisKritaSensorPack *pack = dynamic_cast<const KisKritaSensorPack*>(data.sensorData.constData());
        if (pack) {
            return pack->constSensorsStruct().sensorDistance;
        } else {
            qWarning() << "safeDereferenceDistanceSensor(get): failed to get a Krita sensor data";
            return KisSensorWithLengthData(DistanceId);
        }
    },
    [](KisCurveOptionDataCommon data, KisSensorWithLengthData sensor) -> KisCurveOptionDataCommon {
        KisKritaSensorPack *pack = dynamic_cast<KisKritaSensorPack*>(data.sensorData.data());
        if (pack) {
            pack->sensorsStruct().sensorDistance = sensor;
        } else {
            qWarning() << "safeDereferenceDistanceSensor(set): failed to get a Krita sensor data";
        }
        return data;
    });
}

KisDynamicSensorFactoryDistance::KisDynamicSensorFactoryDistance()
    : KisSimpleDynamicSensorFactory(DistanceId.id(), 0, 1000, i18n("0"), "", "")
{

}

QWidget *KisDynamicSensorFactoryDistance::createConfigWidget(lager::cursor<KisCurveOptionDataCommon> data, QWidget *parent)
{
    QWidget* wdg = new QWidget(parent);
    Ui_SensorDistanceConfiguration stc;
    stc.setupUi(wdg);

    KisSensorWithLengthModel *model =
        new KisSensorWithLengthModel(data.zoom(safeDereferenceDistanceSensor), wdg);

    using namespace KisWidgetConnectionUtils;

    connectControl(stc.checkBoxRepeat, model, "isPeriodic");
    connectControl(stc.spinBoxLength, model, "length");

    stc.spinBoxLength->setSuffix(i18n(" px"));
    stc.spinBoxLength->setExponentRatio(3.0);

    return wdg;
}

int KisDynamicSensorFactoryDistance::maximumValue(int length)
{
    return length >= 0 ? length : KisSimpleDynamicSensorFactory::maximumValue(length);
}

QString KisDynamicSensorFactoryDistance::maximumLabel(int length)
{
    return i18n("%1 px", length);
}
