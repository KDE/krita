/*
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

#ifndef KRA_IMPORT_H_
#define KRA_IMPORT_H_

#include <QVariant>

#include <KisImportExportFilter.h>

class KraImport : public KisImportExportFilter
{
    Q_OBJECT
public:
    KraImport(QObject *parent, const QVariantList &);
    ~KraImport() override;
public:
    ImportExport::ErrorCode convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP configuration = 0) override;
};

#endif
