/*
 *  SPDX-FileCopyrightText: 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_Brush_EXPORT_H_
#define _KIS_Brush_EXPORT_H_

#include <QVariant>
#include <QSpinBox>
#include <QPainter>

#include <KisImportExportFilter.h>
#include <kis_config_widget.h>
#include <kis_properties_configuration.h>


class KisBrushExport : public KisImportExportFilter
{
    Q_OBJECT
public:
    KisBrushExport(QObject *parent, const QVariantList &);
    ~KisBrushExport() override;
    KisImportExportErrorCode convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP configuration = 0) override;
    KisPropertiesConfigurationSP defaultConfiguration(const QByteArray& from = "", const QByteArray& to = "") const override;
    KisConfigWidget *createConfigurationWidget(QWidget *parent, const QByteArray& from = "", const QByteArray& to = "") const override;

    void initializeCapabilities() override;
};

#endif
