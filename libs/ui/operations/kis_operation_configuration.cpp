/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_operation_configuration.h"

KisOperationConfiguration::KisOperationConfiguration()
{
}

KisOperationConfiguration::KisOperationConfiguration(const QString &id)
{
    setProperty("id", id);
}

QString KisOperationConfiguration::id() const
{
    return getString("id", "wrong-id");
}
