/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef ORA_IMPORT_H_
#define ORA_IMPORT_H_

#include <QVariant>

#include <KisImportExportFilter.h>

class OraImport : public KisImportExportFilter
{
    Q_OBJECT
public:
    OraImport(QObject *parent, const QVariantList &);
    ~OraImport() override;
public:
    KisImportExportErrorCode convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP configuration = 0) override;
};

#endif
