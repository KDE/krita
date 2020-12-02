/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2016 Spencer Brown <sbrown655@gmail.com>
 * Copyright (c) 2020 Deif Lou <ginoba@gmail.com>
 * 
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <filter/kis_filter_configuration.h>
#include <KoResourceServerProvider.h>
#include <KoAbstractGradient.h>
#include <KoGradientBackground.h>
#include <KisViewManager.h>
#include <kis_canvas_resource_provider.h>
#include <kis_filter_registry.h>
#include <KoResourceServerAdapter.h>
#include <kis_signals_blocker.h>

#include "KisGradientMapFilterConfigWidget.h"
#include "KisGradientMapFilterConfiguration.h"

KisGradientMapFilterConfigWidget::KisGradientMapFilterConfigWidget(QWidget *parent, Qt::WindowFlags f)
    : KisConfigWidget(parent, f)
    , m_activeGradient(nullptr)
    , m_view(nullptr)
{    
    m_ui.setupUi(this);

    m_gradientChangedCompressor = new KisSignalCompressor(100, KisSignalCompressor::FIRST_ACTIVE);

    KoResourceServerProvider *serverProvider = KoResourceServerProvider::instance();
    QSharedPointer<KoAbstractResourceServerAdapter> gradientResourceAdapter(
                new KoResourceServerAdapter<KoAbstractGradient>(serverProvider->gradientServer()));

    m_gradientPopUp = new KoResourcePopupAction(gradientResourceAdapter,
                                                m_ui.btnGradientChooser);
    m_gradientPopUp->keepAspectRatio(false);
    m_ui.gradientEditor->setGradient(m_activeGradient.data());
    m_ui.gradientEditor->setCompactMode(true);
    m_ui.gradientEditor->setEnabled(true);
    m_ui.btnGradientChooser->setDefaultAction(m_gradientPopUp);
    m_ui.btnGradientChooser->setPopupMode(QToolButton::InstantPopup);
    connect(m_gradientPopUp, SIGNAL(resourceSelected(QSharedPointer<KoShapeBackground>)), this, SLOT(setAbstractGradientToEditor()));
    connect(m_ui.gradientEditor, SIGNAL(sigGradientChanged()), m_gradientChangedCompressor, SLOT(start()));
    connect(m_gradientChangedCompressor, SIGNAL(timeout()), this, SIGNAL(sigConfigurationItemChanged()));

    QObject::connect(m_ui.colorModeComboBox,  QOverload<int>::of(&QComboBox::currentIndexChanged), this, &KisConfigWidget::sigConfigurationItemChanged);
    QObject::connect(m_ui.ditherWidget, &KisDitherWidget::sigConfigurationItemChanged, this, &KisConfigWidget::sigConfigurationItemChanged);
}

KisGradientMapFilterConfigWidget::~KisGradientMapFilterConfigWidget()
{}

void KisGradientMapFilterConfigWidget::setAbstractGradientToEditor()
{
    QSharedPointer<KoGradientBackground> bg =
        qSharedPointerDynamicCast<KoGradientBackground>(
            m_gradientPopUp->currentBackground());
    m_activeGradient = KoStopGradientSP(KoStopGradient::fromQGradient(bg->gradient()));
    m_ui.gradientEditor->setGradient(m_activeGradient.data());

}

KisPropertiesConfigurationSP KisGradientMapFilterConfigWidget::configuration() const
{
    KisGradientMapFilterConfiguration *config = new KisGradientMapFilterConfiguration();

    KoStopGradientSP gradient = KoStopGradientSP(static_cast<KoStopGradient*>(m_activeGradient->clone()));
    if (gradient && m_view) {
        KisCanvasResourceProvider *canvasResourceProvider = m_view->canvasResourceProvider();
        gradient->bakeVariableColors(canvasResourceProvider->fgColor(), canvasResourceProvider->bgColor());
    }
    config->setGradient(m_activeGradient);
    config->setColorMode(m_ui.colorModeComboBox->currentIndex());
    m_ui.ditherWidget->configuration(*config, "dither/");

    return config;
}

void KisGradientMapFilterConfigWidget::setConfiguration(const KisPropertiesConfigurationSP config)
{
    const KisGradientMapFilterConfiguration *filterConfig =
        dynamic_cast<const KisGradientMapFilterConfiguration*>(config.data());
    Q_ASSERT(filterConfig);

    {
        KisSignalsBlocker signalsBlocker(this);

        m_activeGradient = filterConfig->gradient();
        if (m_activeGradient && m_view) {
            KisCanvasResourceProvider *canvasResourceProvider = m_view->canvasResourceProvider();
            m_activeGradient->setVariableColors(canvasResourceProvider->fgColor(), canvasResourceProvider->bgColor());
        }
        m_ui.gradientEditor->setGradient(m_activeGradient.data());

        m_ui.colorModeComboBox->setCurrentIndex(filterConfig->colorMode());

        m_ui.ditherWidget->setConfiguration(*filterConfig, "dither/");
    }

    emit sigConfigurationUpdated();
}

void KisGradientMapFilterConfigWidget::setView(KisViewManager *view)
{
    m_view = view;
}
