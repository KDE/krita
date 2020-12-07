/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2016 Spencer Brown <sbrown655@gmail.com>
 * SPDX-FileCopyrightText: 2020 Deif Lou <ginoba@gmail.com>
 * 
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_GRADIENT_MAP_FILTER_CONFIG_WIDGET_H
#define KIS_GRADIENT_MAP_FILTER_CONFIG_WIDGET_H

#include <kis_config_widget.h>
#include <KoResourcePopupAction.h>
#include <kis_signal_compressor.h>
#include <KoStopGradient.h>

#include "ui_KisGradientMapFilterConfigWidget.h"

class KisViewManager;

class KisGradientMapFilterConfigWidget : public KisConfigWidget
{
    Q_OBJECT

public:
    KisGradientMapFilterConfigWidget(QWidget *parent, Qt::WindowFlags f = Qt::WindowFlags());
    ~KisGradientMapFilterConfigWidget() override;

    KisPropertiesConfigurationSP configuration() const override;
    void setConfiguration(const KisPropertiesConfigurationSP config) override;

    void setView(KisViewManager *view) override;

private:
    Ui_GradientMapFilterConfigWidget m_ui;
    KoResourcePopupAction *m_gradientPopUp;
    KisSignalCompressor *m_gradientChangedCompressor;
    KoStopGradientSP m_activeGradient;
    KisViewManager *m_view;
    
private Q_SLOTS:
    void setAbstractGradientToEditor();
};

#endif
