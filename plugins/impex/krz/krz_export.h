/*
 * SPDX-FileCopyrightText: 2021 Halla Rempt <halla@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _KRZ_EXPORT_H_
#define _KRZ_EXPORT_H_

#include <QVariant>

#include <KisImportExportFilter.h>

class KrzExport : public KisImportExportFilter
{
    Q_OBJECT
public:
    KrzExport(QObject *parent, const QVariantList &);
    ~KrzExport() override;
public:
    KisImportExportErrorCode convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP configuration = 0) override;
    void initializeCapabilities() override;
    QString verify(const QString &fileName) const override;
};

#endif
