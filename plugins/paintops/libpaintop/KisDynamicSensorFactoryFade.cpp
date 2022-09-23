/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisDynamicSensorFactoryFade.h"

#include "ui_SensorFadeConfiguration.h"

#include <KisWidgetConnectionUtils.h>
#include <KisSensorWithLengthModel.h>

KisDynamicSensorFactoryFade::KisDynamicSensorFactoryFade()
    : KisSimpleDynamicSensorFactory(
          0, 1000,
          i18n("0"), "",
          "")
{

}

QWidget *KisDynamicSensorFactoryFade::createConfigWidget(lager::cursor<KisCurveOptionData> data, QWidget *parent)
{
    QWidget* wdg = new QWidget(parent);
    Ui_SensorFadeConfiguration stc;
    stc.setupUi(wdg);

    KisSensorWithLengthModel *model =
        new KisSensorWithLengthModel(data[&KisCurveOptionData::sensorFade], wdg);

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
