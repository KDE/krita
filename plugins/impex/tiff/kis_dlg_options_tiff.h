/*
 *  Copyright (c) 2005-2006 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
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
    ~KisTIFFOptionsWidget();

    void setConfiguration(const KisPropertiesConfigurationSP  cfg);
    KisPropertiesConfigurationSP configuration() const;

private Q_SLOTS:
    void activated(int index);
    void flattenToggled(bool);
    KisTIFFOptions options() const;
public:
    QWidget* wdg;
    Ui_KisWdgOptionsTIFF* optionswdg;
};

#endif
