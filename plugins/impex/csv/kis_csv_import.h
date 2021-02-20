/*
 *  SPDX-FileCopyrightText: 2016 Laszlo Fazekas <mneko@freemail.hu>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef _KIS_CSV_IMPORT_H_
#define _KIS_CSV_IMPORT_H_

#include <QVariant>

#include <KisImportExportFilter.h>

class KisCSVImport : public KisImportExportFilter
{
    Q_OBJECT
public:
    KisCSVImport(QObject *parent, const QVariantList &);
    ~KisCSVImport() override;
public:
    KisImportExportErrorCode convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP configuration = 0) override;
};

#endif
