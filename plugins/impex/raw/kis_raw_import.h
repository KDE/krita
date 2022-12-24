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

class Q_DECL_HIDDEN WdgRawImport : public QWidget, public Ui::WdgRawImport
{
    Q_OBJECT

public:
    WdgRawImport(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        setupUi(this);
    }

    void showEvent(QShowEvent *event) override
    {
        Q_EMIT paint();
        QWidget::showEvent(event);
    }

Q_SIGNALS:
    void paint();
};

namespace KDcrawIface
{
class RawDecodingSettings;
} // namespace KDcrawIface

class KisRawImport : public KisImportExportFilter
{
    Q_OBJECT

public:
    KisRawImport(QObject *parent, const QVariantList &);
    ~KisRawImport() override;

    KisImportExportErrorCode convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP configuration = 0) override;

private Q_SLOTS:
    void slotUpdatePreview();

private:
    KDcrawIface::RawDecodingSettings rawDecodingSettings();

    KoDialog *m_dialog;
    WdgRawImport *m_rawWidget;
};

#endif // KIS_RAW_IMPORT_H_

