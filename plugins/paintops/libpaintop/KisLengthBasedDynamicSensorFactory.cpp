/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisLengthBasedDynamicSensorFactory.h"

KisLengthBasedDynamicSensorFactory::KisLengthBasedDynamicSensorFactory(int minimumValue, int maximumValue, const QString &minimumLabel, std::function<QString (int)> calcMaximumLabel, const QString &valueSuffix)
    : KisSimpleDynamicSensorFactory(minimumValue,
                                    maximumValue,
                                    minimumLabel,
                                    "",
                                    valueSuffix),
      m_calcMaximumLabel(calcMaximumLabel)
{
}

int KisLengthBasedDynamicSensorFactory::maximumValue(int length)
{
    return length >= 0 ? length : KisSimpleDynamicSensorFactory::maximumValue(length);
}

QString KisLengthBasedDynamicSensorFactory::maximumLabel(int length)
{
    return m_calcMaximumLabel(maximumValue(length));
}
