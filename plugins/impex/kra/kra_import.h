/*
 * SPDX-FileCopyrightText: 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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
    KisImportExportErrorCode convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP configuration = 0) override;
};

#endif
