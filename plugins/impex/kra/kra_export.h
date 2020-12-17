/*
 * SPDX-FileCopyrightText: 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _KRA_EXPORT_H_
#define _KRA_EXPORT_H_

#include <QVariant>

#include <KisImportExportFilter.h>

class KraExport : public KisImportExportFilter
{
    Q_OBJECT
public:
    KraExport(QObject *parent, const QVariantList &);
    ~KraExport() override;
public:
    KisImportExportErrorCode convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP configuration = 0) override;
    void initializeCapabilities() override;
    QString verify(const QString &fileName) const override;
};

#endif
