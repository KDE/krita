/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_BMP_EXPORT_H_
#define _KIS_BMP_EXPORT_H_

#include <QVariant>

#include <KisImportExportFilter.h>

class KisQImageIOExport : public KisImportExportFilter
{
    Q_OBJECT
public:
    KisQImageIOExport(QObject *parent, const QVariantList &);
    ~KisQImageIOExport() override;

    KisImportExportErrorCode convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP configuration = 0) override;
    KisConfigWidget *createConfigurationWidget(QWidget *parent, const QByteArray& from, const QByteArray& to) const override;
    KisPropertiesConfigurationSP defaultConfiguration(const QByteArray &, const QByteArray &) const override;
    void initializeCapabilities() override;
};

#endif
