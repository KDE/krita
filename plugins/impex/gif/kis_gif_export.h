/*
 *  SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_GIF_EXPORT_H_
#define _KIS_GIF_EXPORT_H_

#include <QVariant>

#include <KisImportExportFilter.h>

class KisGIFExport : public KisImportExportFilter
{
    Q_OBJECT
public:
    KisGIFExport(QObject *parent, const QVariantList &);
    ~KisGIFExport() override;
public:
    KisImportExportErrorCode convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP configuration = 0) override;
    void initializeCapabilities() override;
};

#endif
