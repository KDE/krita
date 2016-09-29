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

class KisExportCheckBase;
class KisExportConverterBase;

class KRITAIMPEX_EXPORT KisPreExportChecker
{
public:
    KisPreExportChecker();

    void check(KisImageSP image, QMap<QString, KisExportCheckBase *> filterChecks);
    KisImageSP convertedImage(KisImageSP) const;
    QStringList warnings() const;
    bool conversionNeeded() const;

private:
    void initializeChecks();
    QMap<QString, KisExportCheckBase*> m_checks;
    QStringList m_warnings;
    QMap<QString, KisExportConverterBase*> m_conversions;
};

#endif // KISPREEXPORTCHECKER_H
