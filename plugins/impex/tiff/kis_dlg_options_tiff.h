/*
 *  SPDX-FileCopyrightText: 2005-2006 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_DLG_OPTIONS_TIFF_H
#define KIS_DLG_OPTIONS_TIFF_H

#include <kis_tiff_converter.h>
#include <kis_config_widget.h>
#include "ui_kis_wdg_options_tiff.h"

class Ui_KisWdgOptionsTIFF;
/**
    @author Cyrille Berger <cberger@cberger.net>
*/
class KisTIFFOptionsWidget : public KisConfigWidget, public Ui::KisWdgOptionsTIFF
{
    Q_OBJECT
public:
    KisTIFFOptionsWidget(QWidget *parent = 0);

    void setConfiguration(const KisPropertiesConfigurationSP  cfg) override;
    KisPropertiesConfigurationSP configuration() const override;

private Q_SLOTS:
    void activated(int index);
    void flattenToggled(bool);
};

#endif
