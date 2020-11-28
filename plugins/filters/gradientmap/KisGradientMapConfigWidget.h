/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2016 Spencer Brown <sbrown655@gmail.com>
 * SPDX-FileCopyrightText: 2020 Deif Lou <ginoba@gmail.com>
 * 
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
    KoStopGradientSP m_activeGradient;
    void setView(KisViewManager *view) override;
    
private Q_SLOTS:
    void setAbstractGradientToEditor();
};

#endif
