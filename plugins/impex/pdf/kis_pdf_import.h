/*
 *  SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_PDF_IMPORT_H
#define KIS_PDF_IMPORT_H

#include <QVariant>

#include <KisImportExportFilter.h>

class KisPDFImport : public KisImportExportFilter
{
    Q_OBJECT
public:
    KisPDFImport(QObject *parent, const QVariantList &);
    virtual ~KisPDFImport();
public:
    virtual KisImportExportErrorCode convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP configuration = 0);
};

#endif
