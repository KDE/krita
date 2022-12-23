/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISDYNAMICSENSORFACTORY_H
#define KISDYNAMICSENSORFACTORY_H

#include <KisCurveOptionData.h>
#include <lager/cursor.hpp>

class QWidget;

class PAINTOP_EXPORT KisDynamicSensorFactory
{
public:
    virtual ~KisDynamicSensorFactory();

    virtual int minimumValue() = 0;
    virtual int maximumValue(int length) = 0;
    virtual QString minimumLabel() = 0;
    virtual QString maximumLabel(int length) = 0;
    virtual QString valueSuffix() = 0;
    virtual QWidget* createConfigWidget(lager::cursor<KisCurveOptionDataCommon>, QWidget*) = 0;
};

#endif // KISDYNAMICSENSORFACTORY_H
