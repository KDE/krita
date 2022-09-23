/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisDynamicSensorFactoryDistance.h"

#include "ui_SensorDistanceConfiguration.h"

#include <KisWidgetConnectionUtils.h>
#include <KisSensorWithLengthModel.h>

KisDynamicSensorFactoryDistance::KisDynamicSensorFactoryDistance()
    : KisSimpleDynamicSensorFactory(
          0, 1000,
          i18n("0"), "",
          "")
{

}

QWidget *KisDynamicSensorFactoryDistance::createConfigWidget(lager::cursor<KisCurveOptionData> data, QWidget *parent)
{
    QWidget* wdg = new QWidget(parent);
    Ui_SensorDistanceConfiguration stc;
    stc.setupUi(wdg);

    KisSensorWithLengthModel *model =
        new KisSensorWithLengthModel(data[&KisCurveOptionData::sensorDistance], wdg);

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
