/*
 * SPDX-FileCopyrightText: 2023 Rasyuqa A. H. <qampidh@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef DLG_RGBE_EXPORT_H
#define DLG_RGBE_EXPORT_H

#include <QVariant>

#include <KisImportExportFilter.h>
#include <kis_config_widget.h>
#include <kis_meta_data_filter_registry_model.h>

#include "ui_kis_wdg_options_rgbe.h"

class KisWdgOptionsRGBE  : public KisConfigWidget, public Ui::KisWdgOptionsRGBE
{
    Q_OBJECT

public:
    KisWdgOptionsRGBE(QWidget *parent);

    void setConfiguration(const KisPropertiesConfigurationSP cfg) override;
    KisPropertiesConfigurationSP configuration() const override;
};

#endif // DLG_RGBE_EXPORT_H
