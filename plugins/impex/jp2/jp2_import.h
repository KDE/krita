/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.1-only
 */

#ifndef JP2_IMPORT_H_
#define JP2_IMPORT_H_

#include <QVariant>

#include <KisImportExportFilter.h>

class jp2Import : public KisImportExportFilter
{
    Q_OBJECT
public:
    jp2Import(QObject *parent, const QVariantList &);
    virtual ~jp2Import();
public:
    KisImportExportErrorCode convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP configuration = 0) override;
};

#endif
