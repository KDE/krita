/*
 * SPDX-FileCopyrightText: 2023 Rasyuqa A. H. <qampidh@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef RGBE_IMPORT_H_
#define RGBE_IMPORT_H_

#include <KisImportExportFilter.h>

class RGBEImport : public KisImportExportFilter
{
    Q_OBJECT
public:
    RGBEImport(QObject *parent, const QVariantList &);
    ~RGBEImport() override = default;
    bool supportsIO() const override { return true; }

    KisImportExportErrorCode convert(KisDocument *document, QIODevice *io, KisPropertiesConfigurationSP configuration = nullptr) override;
};

#endif // RGBE_IMPORT_H_
