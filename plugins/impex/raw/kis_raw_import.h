/*
 *  SPDX-FileCopyrightText: 2005 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_RAW_IMPORT_H_
#define KIS_RAW_IMPORT_H_

#include <KisImportExportFilter.h>

#include "ui_wdgrawimport.h"

class KoDialog;
class WdgRawImport;

namespace KDcrawIface
{
class RawDecodingSettings;
}

class KisRawImport : public KisImportExportFilter
{
    Q_OBJECT

public:
    KisRawImport(QObject *parent, const QVariantList &);
    ~KisRawImport() override;

public:
    KisImportExportErrorCode convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP configuration = 0) override;


private Q_SLOTS:

    void slotUpdatePreview();
private:
    KDcrawIface::RawDecodingSettings rawDecodingSettings();
private:
    Ui::WdgRawImport m_rawWidget;
    KoDialog* m_dialog;
};

#endif // KIS_RAW_IMPORT_H_

