/*
 *  SPDX-FileCopyrightText: 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_Brush_IMPORT_H_
#define _KIS_Brush_IMPORT_H_

#include <QVariant>

#include <KisImportExportFilter.h>

class KisBrushImport : public KisImportExportFilter
{
    Q_OBJECT
public:
    KisBrushImport(QObject *parent, const QVariantList &);
    ~KisBrushImport() override;
public:
    KisImportExportErrorCode convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP configuration = 0) override;
};

#endif
