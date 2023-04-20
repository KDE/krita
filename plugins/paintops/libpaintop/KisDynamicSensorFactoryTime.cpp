/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisDynamicSensorFactoryTime.h"

#include "ui_SensorTimeConfiguration.h"

#include <KisWidgetConnectionUtils.h>
#include <KisSensorWithLengthModel.h>
#include <KisKritaSensorPack.h>

namespace {
    auto safeDereferenceTimeSensor = lager::lenses::getset(
    [](const KisCurveOptionDataCommon &data) -> KisSensorWithLengthData {
        const KisKritaSensorPack *pack = dynamic_cast<const KisKritaSensorPack*>(data.sensorData.constData());
        if (pack) {
            return pack->constSensorsStruct().sensorTime;
        } else {
            qWarning() << "safeDereferenceTimeSensor(get): failed to get a Krita sensor data";
            return KisSensorWithLengthData(FadeId);
        }
    },
    [](KisCurveOptionDataCommon data, KisSensorWithLengthData sensor) -> KisCurveOptionDataCommon {
        KisKritaSensorPack *pack = dynamic_cast<KisKritaSensorPack*>(data.sensorData.data());
        if (pack) {
            pack->sensorsStruct().sensorTime = sensor;
        } else {
            qWarning() << "safeDereferenceTimeSensor(set): failed to get a Krita sensor data";
        }
        return data;
    });
}

KisDynamicSensorFactoryTime::KisDynamicSensorFactoryTime()
    : KisSimpleDynamicSensorFactory(TimeId.id(), 0, 3000, i18n("0"), "", i18n(" ms"))
{

}

QWidget *KisDynamicSensorFactoryTime::createConfigWidget(lager::cursor<KisCurveOptionDataCommon> data, QWidget *parent)
{
    QWidget* wdg = new QWidget(parent);
    Ui_SensorTimeConfiguration stc;
    stc.setupUi(wdg);

    KisSensorWithLengthModel *model =
        new KisSensorWithLengthModel(data.zoom(safeDereferenceTimeSensor), wdg);

    using namespace KisWidgetConnectionUtils;

    connectControl(stc.checkBoxRepeat, model, "isPeriodic");
    connectControl(stc.spinBoxDuration, model, "length");

    stc.spinBoxDuration->setRange(1, 10000);
    stc.spinBoxDuration->setExponentRatio(2);
    stc.spinBoxDuration->setSuffix(i18n(" ms"));

    return wdg;
}

int KisDynamicSensorFactoryTime::maximumValue(int length)
{
    return length >= 0 ? length : KisSimpleDynamicSensorFactory::maximumValue(length);
}

QString KisDynamicSensorFactoryTime::maximumLabel(int length)
{
    return i18n("%1 ms", length);
}
