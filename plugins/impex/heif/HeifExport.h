/*
 *  Copyright (c) 2018 Dirk Farin <farin@struktur.de>
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

#ifndef HEIF_EXPORT_H_
#define HEIF_EXPORT_H_

#include <QVariant>

#include <KisImportExportFilter.h>
#include <kis_config_widget.h>
#include "ui_WdgHeifExport.h"

class KisWdgOptionsHeif : public KisConfigWidget, public Ui::WdgHeifExport
{
    Q_OBJECT

public:
    KisWdgOptionsHeif(QWidget *parent)
        : KisConfigWidget(parent)
    {
        setupUi(this);
        connect(chkLossless, SIGNAL(toggled(bool)), SLOT(toggleQualitySlider(bool)));
        sliderQuality->setRange(0, 100, 0);
    }

    void setConfiguration(const KisPropertiesConfigurationSP  cfg) override;
    KisPropertiesConfigurationSP configuration() const override;

private Q_SLOTS:

    void toggleQualitySlider(bool toggle);
private:

    bool m_hasAlpha {false};

};


class HeifExport : public KisImportExportFilter
{
    Q_OBJECT
public:
    HeifExport(QObject *parent, const QVariantList &);
    ~HeifExport() override;

    // This should return true if the library can work with a QIODevice, and doesn't want to open the file by itself
    bool supportsIO() const override { return true; }

    ImportExport::ErrorCode convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP configuration = 0) override;
    KisPropertiesConfigurationSP defaultConfiguration(const QByteArray& from = "", const QByteArray& to = "") const override;
    KisConfigWidget *createConfigurationWidget(QWidget *parent, const QByteArray& from = "", const QByteArray& to = "") const override;
    void initializeCapabilities() override;

};

#endif
