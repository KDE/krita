/*
 *  Copyright (c) 2012 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
