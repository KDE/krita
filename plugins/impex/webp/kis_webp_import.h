/*
 *  SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_WEBP_IMPORT_H_
#define _KIS_WEBP_IMPORT_H_

#include <QScopedPointer>
#include <QVariant>

#include <KisImportExportFilter.h>
#include <KoDialog.h>

#include "dlg_webp_import.h"

class KisWebPImport : public KisImportExportFilter
{
    Q_OBJECT
public:
    KisWebPImport(QObject *parent, const QVariantList &);
    ~KisWebPImport() override;

    KisImportExportErrorCode convert(KisDocument *document, QIODevice *io, KisPropertiesConfigurationSP configuration = 0) override;
    KisPropertiesConfigurationSP defaultConfiguration(const QByteArray &from, const QByteArray &to) const override;

private:
    QScopedPointer<KisDlgWebPImport> m_dialog {nullptr};
};

#endif // _KIS_WEBP_IMPORT_H_
