/*
 *  Copyright (c) 2014 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_HeightMap_EXPORT_H_
#define _KIS_HeightMap_EXPORT_H_

#include <QVariant>

#include <KisImportExportFilter.h>
#include <kis_config_widget.h>
#include "ui_kis_wdg_options_heightmap.h"

class KisWdgOptionsHeightmap : public KisConfigWidget, public Ui::WdgOptionsHeightMap
{
    Q_OBJECT

public:
    KisWdgOptionsHeightmap(QWidget *parent)
        : KisConfigWidget(parent)
    {
        setupUi(this);
    }

    void setConfiguration(const KisPropertiesConfigurationSP  cfg);
    KisPropertiesConfigurationSP configuration() const;
};


class KisHeightMapExport : public KisImportExportFilter
{
    Q_OBJECT
public:
    KisHeightMapExport(QObject *parent, const QVariantList &);
    virtual ~KisHeightMapExport();
    KisPropertiesConfigurationSP defaultConfiguration(const QByteArray& from = "", const QByteArray& to = "") const;
    KisPropertiesConfigurationSP lastSavedConfiguration(const QByteArray &from = "", const QByteArray &to = "") const;
    KisConfigWidget *createConfigurationWidget(QWidget *parent, const QByteArray& from = "", const QByteArray& to = "") const;
public:
    virtual KisImportExportFilter::ConversionStatus convert(const QByteArray& from, const QByteArray& to, KisPropertiesConfigurationSP configuration = 0);
};

#endif
