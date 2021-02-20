/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KisExportCheckBase.h"

KisExportCheckBase::KisExportCheckBase(const QString &id, Level level, const QString &customWarning, bool _perLayerCheck)
    : m_id(id)
    , m_level(level)
    , m_perLayerCheck(_perLayerCheck)
{
    if (!customWarning.isEmpty()) {
        m_warning = customWarning;
    }
}

KisExportCheckBase::~KisExportCheckBase()
{
}

QString KisExportCheckBase::id() const
{
    return m_id;
}

bool KisExportCheckBase::perLayerCheck() const
{
    return m_perLayerCheck;
}

QString KisExportCheckBase::warning() const
{
    return m_warning;
}

