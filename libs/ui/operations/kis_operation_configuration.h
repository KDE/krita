/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_OPERATION_CONFIGURATION_H
#define __KIS_OPERATION_CONFIGURATION_H

#include <QString>
#include <kritaui_export.h>
#include "kis_properties_configuration.h"


class KRITAUI_EXPORT KisOperationConfiguration : public KisPropertiesConfiguration
{
public:
    KisOperationConfiguration();
    ~KisOperationConfiguration() override {}
    KisOperationConfiguration(const QString &id);

    QString id() const;
private:
    Q_DISABLE_COPY(KisOperationConfiguration)
};

typedef KisPinnedSharedPtr<KisOperationConfiguration> KisOperationConfigurationSP;

#endif /* __KIS_OPERATION_CONFIGURATION_H */
