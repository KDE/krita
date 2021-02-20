/*
 *  SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef _PSD_EXPORT_H_
#define _PSD_EXPORT_H_

#include <QVariant>

#include <KisImportExportFilter.h>

class psdExport : public KisImportExportFilter {
    Q_OBJECT
    public:
        psdExport(QObject *parent, const QVariantList &);
        ~psdExport() override;
    public:
        KisImportExportErrorCode convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP configuration = 0) override;
        void initializeCapabilities() override;
};

#endif
