/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisSimpleDynamicSensorFactory.h"

KisSimpleDynamicSensorFactory::KisSimpleDynamicSensorFactory(const QString &id,
                                                             int minimumValue,
                                                             int maximumValue,
                                                             const QString &minimumLabel,
                                                             const QString &maximumLabel,
                                                             const QString &valueSuffix)
    : m_minimumValue(minimumValue)
    , m_maximumValue(maximumValue)
    , m_id(id)
    , m_minimumLabel(minimumLabel)
    , m_maximumLabel(maximumLabel)
    , m_valueSuffix(valueSuffix)
{
}

QString KisSimpleDynamicSensorFactory::id() const
{
    return m_id;
}

int KisSimpleDynamicSensorFactory::minimumValue()
{
    return m_minimumValue;
}

int KisSimpleDynamicSensorFactory::maximumValue(int length)
{
    Q_UNUSED(length);
    return m_maximumValue;
}

QString KisSimpleDynamicSensorFactory::minimumLabel()
{
    return m_minimumLabel;
}

QString KisSimpleDynamicSensorFactory::maximumLabel(int length)
{
    Q_UNUSED(length);
    return m_maximumLabel;
}

QString KisSimpleDynamicSensorFactory::valueSuffix()
{
    return m_valueSuffix;
}

QWidget *KisSimpleDynamicSensorFactory::createConfigWidget(lager::cursor<KisCurveOptionDataCommon>, QWidget *)
{
    return nullptr;
}
