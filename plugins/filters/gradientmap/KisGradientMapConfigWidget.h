/*
 * This file is part of Krita
 *
 * Copyright (c) 2016 Spencer Brown <sbrown655@gmail.com>
 * Copyright (c) 2020 Deif Lou <ginoba@gmail.com>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_GRADIENT_MAP_CONFIG_WIDGET_H
#define KIS_GRADIENT_MAP_CONFIG_WIDGET_H

//#include <filter/kis_color_transformation_configuration.h>
#include <kis_config_widget.h>
#include <KoResourcePopupAction.h>
#include <kis_signal_compressor.h>
#include <KoStopGradient.h>

#include "ui_KisGradientMapConfigWidget.h"

class KisGradientMapConfigWidget : public KisConfigWidget
{
    Q_OBJECT
public:
    KisGradientMapConfigWidget(QWidget *parent, KisPaintDeviceSP dev, Qt::WindowFlags f = Qt::WindowFlags());
    ~KisGradientMapConfigWidget() override;

    KisPropertiesConfigurationSP configuration() const override;
    void setConfiguration(const KisPropertiesConfigurationSP config) override;

private:
    Ui_GradientMapConfigWidget m_ui;
    KoResourcePopupAction *m_gradientPopUp;
    KisSignalCompressor *m_gradientChangedCompressor;
    KoStopGradient *m_activeGradient;
    void setView(KisViewManager *view) override;
    
private Q_SLOTS:
    void setAbstractGradientToEditor();
};

#endif
