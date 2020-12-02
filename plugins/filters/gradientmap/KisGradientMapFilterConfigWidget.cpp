/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2016 Spencer Brown <sbrown655@gmail.com>
 * Copyright (c) 2020 Deif Lou <ginoba@gmail.com>
 * 
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <filter/kis_filter_configuration.h>
#include <KisViewManager.h>
#include <kis_canvas_resource_provider.h>
#include <kis_signals_blocker.h>
#include <KisGradientConversion.h>

#include "KisGradientMapFilterConfigWidget.h"
#include "KisGradientMapFilterConfiguration.h"

KisGradientMapFilterConfigWidget::KisGradientMapFilterConfigWidget(QWidget *parent, Qt::WindowFlags f)
    : KisConfigWidget(parent, f)
    , m_view(nullptr)
{    
    m_ui.setupUi(this);

    m_gradientChangedCompressor = new KisSignalCompressor(50, KisSignalCompressor::FIRST_ACTIVE);

    m_ui.widgetGradientEditor->setContentsMargins(10, 10, 10, 10);
    m_ui.widgetGradientEditor->loadUISettings();
    
    connect(m_ui.widgetGradientEditor, SIGNAL(sigGradientChanged()), m_gradientChangedCompressor, SLOT(start()));
    connect(m_gradientChangedCompressor, SIGNAL(timeout()), this, SIGNAL(sigConfigurationItemChanged()));

    connect(m_ui.comboBoxColorMode, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &KisConfigWidget::sigConfigurationItemChanged);
    connect(m_ui.widgetDither, SIGNAL(sigConfigurationItemChanged()), this, SIGNAL(sigConfigurationItemChanged()));
}

KisGradientMapFilterConfigWidget::~KisGradientMapFilterConfigWidget()
{
    m_ui.widgetGradientEditor->saveUISettings();
}

KisPropertiesConfigurationSP KisGradientMapFilterConfigWidget::configuration() const
{
    KisGradientMapFilterConfiguration *config = new KisGradientMapFilterConfiguration();
    
    KoAbstractGradientSP gradient = m_ui.widgetGradientEditor->gradient();
    if (gradient && m_view) {
        KisCanvasResourceProvider *canvasResourceProvider = m_view->canvasResourceProvider();
        gradient->bakeVariableColors(canvasResourceProvider->fgColor(), canvasResourceProvider->bgColor());
    }
    config->setGradient(gradient);

    config->setColorMode(m_ui.comboBoxColorMode->currentIndex());
    m_ui.widgetDither->configuration(*config, "dither/");

    return config;
}

void KisGradientMapFilterConfigWidget::setConfiguration(const KisPropertiesConfigurationSP config)
{
    const KisGradientMapFilterConfiguration *filterConfig =
        dynamic_cast<const KisGradientMapFilterConfiguration*>(config.data());
    Q_ASSERT(filterConfig);

    {
        KisSignalsBlocker signalsBlocker(this);

        m_ui.widgetGradientEditor->setGradient(filterConfig->gradient());
        m_ui.comboBoxColorMode->setCurrentIndex(filterConfig->colorMode());
        m_ui.widgetDither->setConfiguration(*filterConfig, "dither/");
    }

    emit sigConfigurationUpdated();
}

void KisGradientMapFilterConfigWidget::setView(KisViewManager *view)
{
    m_view = view;
    if (view) {
        KisCanvasResourceProvider *canvasResourceProvider = m_view->canvasResourceProvider();
        m_ui.widgetGradientEditor->setVariableColors(canvasResourceProvider->fgColor(), canvasResourceProvider->bgColor());
    }
}
