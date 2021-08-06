/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_WEBP_EXPORT_H_
#define _KIS_WEBP_EXPORT_H_

#include <QVariant>

#include <KisImportExportFilter.h>

class KisWebPExport : public KisImportExportFilter
{
    Q_OBJECT
public:
    KisWebPExport(QObject *parent, const QVariantList &);
    ~KisWebPExport() override;

    KisImportExportErrorCode convert(KisDocument *document, QIODevice *io, KisPropertiesConfigurationSP configuration = 0) override;
    KisPropertiesConfigurationSP defaultConfiguration(const QByteArray &from, const QByteArray &to) const override;
    KisConfigWidget *createConfigurationWidget(QWidget *parent, const QByteArray &from = "", const QByteArray &to = "") const override;
    void initializeCapabilities() override;
};

#endif
