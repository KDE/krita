/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_TGA_IMPORT_H_
#define _KIS_TGA_IMPORT_H_

#include <QVariant>

#include <KisImportExportFilter.h>

class KisTGAImport : public KisImportExportFilter
{
    Q_OBJECT
public:
    KisTGAImport(QObject *parent, const QVariantList &);
    ~KisTGAImport() override;
public:
    KisImportExportErrorCode convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP configuration = 0) override;
};

#endif
