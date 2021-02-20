/*
 *  SPDX-FileCopyrightText: 2016 Laszlo Fazekas <mneko@freemail.hu>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_CSV_EXPORT_H_
#define _KIS_CSV_EXPORT_H_

#include <QVariant>

#include <KisImportExportFilter.h>

class KisCSVExport : public KisImportExportFilter
{
    Q_OBJECT
public:
    KisCSVExport(QObject *parent, const QVariantList &);
    ~KisCSVExport() override;
public:
    KisImportExportErrorCode convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP configuration = 0) override;
    void initializeCapabilities() override;
};

#endif
