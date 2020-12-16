/*
 *  SPDX-FileCopyrightText: 2005 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_JPEG_EXPORT_H_
#define _KIS_JPEG_EXPORT_H_

#include <QVariant>

#include <KisImportExportFilter.h>
#include <kis_config_widget.h>
#include "ui_kis_wdg_options_jpeg.h"
#include <kis_meta_data_store.h>
#include <kis_meta_data_filter_registry_model.h>


class KisWdgOptionsJPEG : public KisConfigWidget, public Ui::WdgOptionsJPEG
{
    Q_OBJECT

public:
    KisWdgOptionsJPEG(QWidget *parent);
    void setConfiguration(const KisPropertiesConfigurationSP  cfg) override;
    KisPropertiesConfigurationSP configuration() const override;
private:
    KisMetaData::FilterRegistryModel m_filterRegistryModel;
};


class KisJPEGExport : public KisImportExportFilter
{
    Q_OBJECT
public:
    KisJPEGExport(QObject *parent, const QVariantList &);
    ~KisJPEGExport() override;
public:
    KisImportExportErrorCode convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP configuration = 0) override;
    KisPropertiesConfigurationSP defaultConfiguration(const QByteArray& from = "", const QByteArray& to = "") const override;
    KisConfigWidget *createConfigurationWidget(QWidget *parent, const QByteArray& from = "", const QByteArray& to = "") const override;
    void initializeCapabilities() override;
};

#endif
