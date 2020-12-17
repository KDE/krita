/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_TGA_EXPORT_H_
#define _KIS_TGA_EXPORT_H_

#include <QVariant>

#include <KisImportExportFilter.h>

class KisTGAExport : public KisImportExportFilter
{
    Q_OBJECT
public:
    KisTGAExport(QObject *parent, const QVariantList &);
    ~KisTGAExport() override;
public:
    KisImportExportErrorCode convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP configuration = 0) override;
    void initializeCapabilities() override;
};

#endif
