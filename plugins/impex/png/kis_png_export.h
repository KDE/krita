/*
 *  SPDX-FileCopyrightText: 2005 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_PNG_EXPORT_H_
#define _KIS_PNG_EXPORT_H_

#include "ui_kis_wdg_options_png.h"

#include <KisImportExportFilter.h>
#include <kis_config_widget.h>

class KisWdgOptionsPNG : public KisConfigWidget, public Ui::KisWdgOptionsPNG
{
    Q_OBJECT

public:
    KisWdgOptionsPNG(QWidget *parent);

    void setConfiguration(const KisPropertiesConfigurationSP  config) override;
    KisPropertiesConfigurationSP configuration() const override;

private Q_SLOTS:
    void on_alpha_toggled(bool checked);
    void slotUseHDRChanged(bool value);
};

class KisPNGExport : public KisImportExportFilter
{
    Q_OBJECT

public:

    KisPNGExport(QObject *parent, const QVariantList &);
    ~KisPNGExport() override;
public:
    KisImportExportErrorCode convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP configuration = 0) override;

    KisPropertiesConfigurationSP defaultConfiguration(const QByteArray& from = "", const QByteArray& to = "") const override;
    KisConfigWidget *createConfigurationWidget(QWidget *parent, const QByteArray& from = "", const QByteArray& to = "") const override;
    void initializeCapabilities() override;
};

#endif
