/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISDYNAMICSENSORFACTORYDISTANCE_H
#define KISDYNAMICSENSORFACTORYDISTANCE_H

#include "KisSimpleDynamicSensorFactory.h"

class PAINTOP_EXPORT KisDynamicSensorFactoryDistance : public KisSimpleDynamicSensorFactory
{
public:
    KisDynamicSensorFactoryDistance();
    QWidget* createConfigWidget(lager::cursor<KisCurveOptionDataCommon> data, QWidget*parent) override;

    int maximumValue(int length) override;
    QString maximumLabel(int length) override;
};


#endif // KISDYNAMICSENSORFACTORYDISTANCE_H
