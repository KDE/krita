/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef _KIS_XCF_IMPORT_H_
#define _KIS_XCF_IMPORT_H_

#include <QVariant>
#include <QIODevice>

#include <KisImportExportFilter.h>

class KisDocument;

class KisXCFImport : public KisImportExportFilter
{
    Q_OBJECT
public:
    KisXCFImport(QObject *parent, const QVariantList &);
    ~KisXCFImport() override;
public:
    KisImportExportErrorCode convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP configuration = 0) override;
};

#endif
