/*
 *  Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
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
    KisImportExportFilter::ConversionStatus convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP configuration = 0) override;

    KisPropertiesConfigurationSP defaultConfiguration(const QByteArray& from = "", const QByteArray& to = "") const override;
    KisConfigWidget *createConfigurationWidget(QWidget *parent, const QByteArray& from = "", const QByteArray& to = "") const override;
    void initializeCapabilities() override;
};

#endif
