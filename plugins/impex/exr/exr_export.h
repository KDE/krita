/*
 *  SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _EXR_EXPORT_H_
#define _EXR_EXPORT_H_

#include <QVariant>

#include <KisImportExportFilter.h>
#include <kis_config_widget.h>
#include "ui_exr_export_widget.h"

class KisWdgOptionsExr : public KisConfigWidget, public Ui::ExrExportWidget
{
    Q_OBJECT

public:
    KisWdgOptionsExr(QWidget *parent)
        : KisConfigWidget(parent)
    {
        setupUi(this);
    }

    void setConfiguration(const KisPropertiesConfigurationSP  cfg) override;
    KisPropertiesConfigurationSP configuration() const override;
};


class EXRExport : public KisImportExportFilter
{
    Q_OBJECT
public:
    EXRExport(QObject *parent, const QVariantList &);
    ~EXRExport() override;
    bool supportsIO() const override { return false; }
    KisImportExportErrorCode convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP configuration = 0) override;
    KisPropertiesConfigurationSP defaultConfiguration(const QByteArray& from = "", const QByteArray& to = "") const override;
    KisConfigWidget *createConfigurationWidget(QWidget *parent, const QByteArray& from = "", const QByteArray& to = "") const override;
    void initializeCapabilities() override;

};

#endif
