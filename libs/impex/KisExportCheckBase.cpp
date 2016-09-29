/* This file is part of the KDE project
 * Copyright (C) 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KisExportCheckBase.h"
#include "KisExportConverterBase.h"

KisExportCheckBase::KisExportCheckBase(const QString &id, Level level, const QString &customWarning)
    : m_id(id)
    , m_level(level)
    , m_converter(0)
{
    if (!customWarning.isEmpty()) {
        m_warning = customWarning;
    }
}

KisExportCheckBase::~KisExportCheckBase()
{
    delete m_converter;
}

QString KisExportCheckBase::id() const
{
    return m_id;
}

KisExportConverterBase *KisExportCheckBase::converter() const
{
    return m_converter;
}

QString KisExportCheckBase::warning() const
{
    return m_warning;
}

