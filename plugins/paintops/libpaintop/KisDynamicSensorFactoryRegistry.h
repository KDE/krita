/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISDYNAMICSENSORFACTORYREGISTRY_H
#define KISDYNAMICSENSORFACTORYREGISTRY_H

#include "KoGenericRegistry.h"
#include "KisDynamicSensorFactory.h"
#include "kritapaintop_export.h"

class PAINTOP_EXPORT KisDynamicSensorFactoryRegistry : public KoGenericRegistry<KisDynamicSensorFactory*>
{
public:
    KisDynamicSensorFactoryRegistry();
    ~KisDynamicSensorFactoryRegistry();

    static KisDynamicSensorFactoryRegistry* instance();

private:
    void addImpl(const KoID &id,
                 int minimumValue,
                 int maximumValue,
                 const QString &minimumLabel,
                 const QString &maximumLabel,
                 const QString &valueSuffix);
};

#endif // KISDYNAMICSENSORFACTORYREGISTRY_H
