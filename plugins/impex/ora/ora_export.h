/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _ORA_EXPORT_H_
#define _ORA_EXPORT_H_

#include <QVariant>

#include <KisImportExportFilter.h>

class OraExport : public KisImportExportFilter
{
    Q_OBJECT
public:
    OraExport(QObject *parent, const QVariantList &);
    ~OraExport() override;
public:
    KisImportExportErrorCode convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP configuration = 0) override;
    void initializeCapabilities() override;
    QString verify(const QString &fileName) const override;
};

#endif
