/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISDYNAMICSENSORFACTORYTIME_H
#define KISDYNAMICSENSORFACTORYTIME_H

#include "KisSimpleDynamicSensorFactory.h"

class PAINTOP_EXPORT KisDynamicSensorFactoryTime : public KisSimpleDynamicSensorFactory
{
public:
    KisDynamicSensorFactoryTime();
    QWidget* createConfigWidget(lager::cursor<KisCurveOptionDataCommon> data, QWidget*parent) override;

    int maximumValue(int length) override;
    QString maximumLabel(int length)override;
};

#endif // KISDYNAMICSENSORFACTORYTIME_H
