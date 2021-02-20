/*
 *  SPDX-FileCopyrightText: 2005 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef _KIS_TIFF_IMPORT_H_
#define _KIS_TIFF_IMPORT_H_

#include <QVariant>

#include <KisImportExportFilter.h>

class KisTIFFImport : public KisImportExportFilter
{
    Q_OBJECT
public:
    KisTIFFImport(QObject *parent, const QVariantList &);
    ~KisTIFFImport() override;
    bool supportsIO() const override { return false; }
public:
    KisImportExportErrorCode convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP configuration = 0) override;
};

#endif
