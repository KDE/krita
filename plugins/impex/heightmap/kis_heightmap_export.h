/*
 *  SPDX-FileCopyrightText: 2014 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2017 Victor Wåhlström <victor.wahlstrom@initiali.se>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _KIS_HeightMap_EXPORT_H_
#define _KIS_HeightMap_EXPORT_H_

#include <QVariant>

#include <KisImportExportFilter.h>
#include <kis_config_widget.h>

class KisHeightMapExport : public KisImportExportFilter
{
    Q_OBJECT
public:
    KisHeightMapExport(QObject *parent, const QVariantList &);
    ~KisHeightMapExport() override;
    KisPropertiesConfigurationSP defaultConfiguration(const QByteArray& from = "", const QByteArray& to = "") const override;
    KisConfigWidget *createConfigurationWidget(QWidget *parent, const QByteArray& from = "", const QByteArray& to = "") const override;
    void initializeCapabilities() override;
    
    KisImportExportErrorCode convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP configuration = 0) override;
};

#endif
