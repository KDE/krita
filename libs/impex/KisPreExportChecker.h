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
#ifndef KISPREEXPORTCHECKER_H
#define KISPREEXPORTCHECKER_H

#include <kis_types.h>
#include "kritaimpex_export.h"

#include "KisExportCheckBase.h"

class KisExportConverterBase;

class KRITAIMPEX_EXPORT KisPreExportChecker
{
public:
    KisPreExportChecker();

    /**
     * @brief check checks the image against the capabilities of the export filter
     * @param image the current image
     * @param filterChecks the list of capabilities the filter possesses
     * @return true if no warnings and no conversions are needed
     */
    bool check(KisImageSP image, QMap<QString, KisExportCheckBase *> filterChecks);
    QStringList warnings() const;
    QStringList errors() const;

private:

    QStringList m_errors;
    QStringList m_warnings;
};

#endif // KISPREEXPORTCHECKER_H
