/*
 * This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2016 Spencer Brown <sbrown655@gmail.com>
 * SPDX-FileCopyrightText: 2020 Deif Lou <ginoba@gmail.com>
 * 
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <filter/kis_filter_configuration.h>
#include <KoResourceServerProvider.h>
#include <KoAbstractGradient.h>
#include <KoGradientBackground.h>
#include <KisGlobalResourcesInterface.h>
#include <KisViewManager.h>
#include <kis_canvas_resource_provider.h>
#include <kis_filter_registry.h>
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

    m_gradientPopUp = new KoResourcePopupAction(ResourceType::Gradients, 0, m_ui.btnGradientChooser);

    m_ui.gradientEditor->setGradient(m_activeGradient);
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
    m_activeGradient = KoStopGradient::fromQGradient(bg->gradient());
    m_ui.gradientEditor->setGradient(m_activeGradient);
}

KisPropertiesConfigurationSP KisGradientMapFilterConfigWidget::configuration() const
{
    KisGradientMapFilterConfiguration *config = new KisGradientMapFilterConfiguration(KisGlobalResourcesInterface::instance());
    
    KoStopGradientSP gradient = m_activeGradient->clone().dynamicCast<KoStopGradient>();
    if (gradient && m_view) {
        KoCanvasResourcesInterfaceSP canvasResourcesInterface =
            m_view->canvasResourceProvider()->resourceManager()->canvasResourcesInterface();
        gradient->bakeVariableColors(canvasResourcesInterface);
    }
    config->setGradient(gradient);

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
            KoCanvasResourcesInterfaceSP canvasResourcesInterface =
                m_view->canvasResourceProvider()->resourceManager()->canvasResourcesInterface();
            m_activeGradient->updateVariableColors(canvasResourcesInterface);
        }
        m_ui.gradientEditor->setGradient(m_activeGradient);
        
        m_ui.colorModeComboBox->setCurrentIndex(filterConfig->colorMode());

        m_ui.ditherWidget->setConfiguration(*filterConfig, "dither/");
    }

    emit sigConfigurationUpdated(); 
}

void KisGradientMapFilterConfigWidget::setView(KisViewManager *view)
{
    m_view = view;
}
