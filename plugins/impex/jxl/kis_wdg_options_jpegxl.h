/*
 * SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef DLG_JPEGXL_EXPORT_H
#define DLG_JPEGXL_EXPORT_H

#include <QVariant>

#include <KisImportExportFilter.h>
#include <kis_config_widget.h>
#include <kis_meta_data_filter_registry_model.h>

#include "ui_kis_wdg_options_jpegxl.h"

class KisWdgOptionsJPEGXL : public KisConfigWidget, public Ui::KisWdgOptionsJPEGXL
{
    Q_OBJECT

public:
    KisWdgOptionsJPEGXL(QWidget *parent);

    void setConfiguration(const KisPropertiesConfigurationSP cfg) override;
    KisPropertiesConfigurationSP configuration() const override;

private:
    KisMetaData::FilterRegistryModel m_filterRegistryModel;
};

#endif // DLG_JPEGXL_EXPORT_H
