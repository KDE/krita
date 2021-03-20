/*
 *  SPDX-FileCopyrightText: 2005 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef _KIS_JPEG_IMPORT_H_
#define _KIS_JPEG_IMPORT_H_

#include <QVariant>

#include <KisImportExportFilter.h>

class KisJPEGImport : public KisImportExportFilter
{
    Q_OBJECT
public:
    KisJPEGImport(QObject *parent, const QVariantList &);
    ~KisJPEGImport() override;
public:
    KisImportExportErrorCode convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP configuration = 0) override;
};

#endif
