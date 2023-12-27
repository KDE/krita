/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisDynamicSensorFactoryFade.h"

#include "ui_SensorFadeConfiguration.h"

#include <KisWidgetConnectionUtils.h>
#include <KisSensorWithLengthModel.h>
#include <KisKritaSensorPack.h>

namespace {
    auto safeDereferenceFadeSensor = lager::lenses::getset(
    [](const KisCurveOptionDataCommon &data) -> KisSensorWithLengthData {
        const KisKritaSensorPack *pack = dynamic_cast<const KisKritaSensorPack*>(data.sensorData.constData());
        if (pack) {
            return pack->constSensorsStruct().sensorFade;
        } else {
            qWarning() << "safeDereferenceFadeSensor(get): failed to get a Krita sensor data";
            return KisSensorWithLengthData(FadeId);
        }
    },
    [](KisCurveOptionDataCommon data, KisSensorWithLengthData sensor) -> KisCurveOptionDataCommon {
        KisKritaSensorPack *pack = dynamic_cast<KisKritaSensorPack*>(data.sensorData.data());
        if (pack) {
            pack->sensorsStruct().sensorFade = sensor;
        } else {
            qWarning() << "safeDereferenceFadeSensor(set): failed to get a Krita sensor data";
        }
        return data;
    });
}

KisDynamicSensorFactoryFade::KisDynamicSensorFactoryFade()
    : KisSimpleDynamicSensorFactory(FadeId.id(), 0, 1000, i18n("0"), "", "")
{

}

QWidget *KisDynamicSensorFactoryFade::createConfigWidget(lager::cursor<KisCurveOptionDataCommon> data, QWidget *parent)
{
    QWidget* wdg = new QWidget(parent);
    Ui_SensorFadeConfiguration stc;
    stc.setupUi(wdg);

    KisSensorWithLengthModel *model =
        new KisSensorWithLengthModel(data.zoom(safeDereferenceFadeSensor), wdg);

    using namespace KisWidgetConnectionUtils;

    connectControl(stc.checkBoxRepeat, model, "isPeriodic");
    connectControl(stc.spinBoxLength, model, "length");

    stc.spinBoxLength->setExponentRatio(3.0);

    return wdg;
}

int KisDynamicSensorFactoryFade::maximumValue(int length)
{
    return length >= 0 ? length : KisSimpleDynamicSensorFactory::maximumValue(length);
}

QString KisDynamicSensorFactoryFade::maximumLabel(int length)
{
    return i18n("%1", length);
}
