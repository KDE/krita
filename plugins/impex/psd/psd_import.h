/*
 *  SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef PSD_IMPORT_H_
#define PSD_IMPORT_H_

#include <QVariant>

#include <KisImportExportFilter.h>

class psdImport : public KisImportExportFilter {
    Q_OBJECT
    public:
        psdImport(QObject *parent, const QVariantList &);
        ~psdImport() override;
    public:
        KisImportExportErrorCode convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP configuration = 0) override;
};

#endif
