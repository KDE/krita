/*
 *  SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef JPEG_XL_IMPORT_H_
#define JPEG_XL_IMPORT_H_

#include <KisImportExportFilter.h>

class JPEGXLImport : public KisImportExportFilter
{
    Q_OBJECT
public:
    JPEGXLImport(QObject *parent, const QVariantList &);
    ~JPEGXLImport() override = default;
    bool supportsIO() const override { return true; }

    KisImportExportErrorCode convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP configuration = nullptr) override;
};

#endif
