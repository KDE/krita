/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISSIMPLEDYNAMICSENSORFACTORY_H
#define KISSIMPLEDYNAMICSENSORFACTORY_H

#include "KisDynamicSensorFactory.h"

#include "KisCurveOptionDataCommon.h"

class PAINTOP_EXPORT KisSimpleDynamicSensorFactory : public KisDynamicSensorFactory
{
public:
    KisSimpleDynamicSensorFactory(const QString &id,
                                  int minimumValue,
                                  int maximumValue,
                                  const QString &minimumLabel,
                                  const QString &maximumLabel,
                                  const QString &valueSuffix);

    QString id() const override;
    int minimumValue() override;
    int maximumValue(int length) override;
    QString minimumLabel() override;
    QString maximumLabel(int length) override;
    QString valueSuffix() override;
    QWidget* createConfigWidget(lager::cursor<KisCurveOptionDataCommon>, QWidget*) override;

    int m_minimumValue;
    int m_maximumValue;
    QString m_id;
    QString m_minimumLabel;
    QString m_maximumLabel;
    QString m_valueSuffix;
};

#endif // KISSIMPLEDYNAMICSENSORFACTORY_H
