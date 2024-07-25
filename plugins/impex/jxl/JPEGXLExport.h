/*
 *  SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef JPEG_XL_EXPORT_H_
#define JPEG_XL_EXPORT_H_

#include <KisImportExportFilter.h>

class JPEGXLExport : public KisImportExportFilter
{
    Q_OBJECT
public:
    JPEGXLExport(QObject *parent, const QVariantList &);
    ~JPEGXLExport() override = default;

    KisImportExportErrorCode
    convert(KisDocument *document, QIODevice *io, KisPropertiesConfigurationSP cfg = nullptr) override;
    KisPropertiesConfigurationSP defaultConfiguration(const QByteArray &from = "",
                                                      const QByteArray &to = "") const override;
    KisConfigWidget *
    createConfigurationWidget(QWidget *parent, const QByteArray &from = "", const QByteArray &to = "") const override;
    void initializeCapabilities() override;
};

#endif
