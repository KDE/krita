/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef DLG_WEBP_EXPORT_H
#define DLG_WEBP_EXPORT_H

#include <kis_config_widget.h>

#include "ui_kis_wdg_options_webp.h"

class KisWdgOptionsWebP : public KisConfigWidget, public Ui::KisWdgOptionsWebP
{
    Q_OBJECT

public:
    KisWdgOptionsWebP(QWidget *parent);
    void setConfiguration(const KisPropertiesConfigurationSP cfg) override;
    KisPropertiesConfigurationSP configuration() const override;
private Q_SLOTS:
    void changePreset();
};

#endif // DLG_WEBP_EXPORT_H
