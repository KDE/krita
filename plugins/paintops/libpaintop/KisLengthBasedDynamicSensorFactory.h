/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISLENGTHBASEDDYNAMICSENSORFACTORY_H
#define KISLENGTHBASEDDYNAMICSENSORFACTORY_H

#include <KisSimpleDynamicSensorFactory.h>

class PAINTOP_EXPORT KisLengthBasedDynamicSensorFactory : public KisSimpleDynamicSensorFactory
{
public:
    KisLengthBasedDynamicSensorFactory(int minimumValue,
                                       int maximumValue,
                                       const QString &minimumLabel,
                                       std::function<QString(int)> calcMaximumLabel,
                                       const QString &valueSuffix);

    int maximumValue(int length) override;
    QString maximumLabel(int length) override;

private:
    std::function<QString(int)> m_calcMaximumLabel;
};

#endif // KISLENGTHBASEDDYNAMICSENSORFACTORY_H
