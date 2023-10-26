/*
 * SPDX-FileCopyrightText: 2023 Rasyuqa A. H. <qampidh@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef RGBE_EXPORT_H
#define RGBE_EXPORT_H

#include <KisImportExportFilter.h>

class RGBEExport : public KisImportExportFilter
{
    Q_OBJECT
public:
    RGBEExport(QObject *parent, const QVariantList &);
    ~RGBEExport() override = default;

    KisImportExportErrorCode
    convert(KisDocument *document, QIODevice *io, KisPropertiesConfigurationSP cfg = nullptr) override;
    KisPropertiesConfigurationSP defaultConfiguration(const QByteArray &from = "",
                                                      const QByteArray &to = "") const override;
    KisConfigWidget *
    createConfigurationWidget(QWidget *parent, const QByteArray &from = "", const QByteArray &to = "") const override;
    void initializeCapabilities() override;
};

#endif // RGBE_EXPORT_H
