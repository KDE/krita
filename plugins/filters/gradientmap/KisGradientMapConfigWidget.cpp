/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2016 Spencer Brown <sbrown655@gmail.com>
 * Copyright (c) 2020 Deif Lou <ginoba@gmail.com>
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

#include "KisGradientMapConfigWidget.h"

KisGradientMapConfigWidget::KisGradientMapConfigWidget(QWidget *parent, KisPaintDeviceSP dev, Qt::WindowFlags f)
    : KisConfigWidget(parent, f)
{
    Q_UNUSED(dev);
    
    m_ui.setupUi(this);

    m_gradientChangedCompressor = new KisSignalCompressor(100, KisSignalCompressor::FIRST_ACTIVE);

    m_gradientPopUp = new KoResourcePopupAction(ResourceType::Gradients, 0, m_ui.btnGradientChooser);

    m_activeGradient = KoStopGradient::fromQGradient(m_gradientPopUp->currentResource().dynamicCast<KoAbstractGradient>()->toQGradient());
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

KisGradientMapConfigWidget::~KisGradientMapConfigWidget()
{}

void KisGradientMapConfigWidget::setAbstractGradientToEditor()
{
    QSharedPointer<KoGradientBackground> bg =
        qSharedPointerDynamicCast<KoGradientBackground>(
            m_gradientPopUp->currentBackground());
    m_activeGradient = KoStopGradient::fromQGradient(bg->gradient());
    m_ui.gradientEditor->setGradient(m_activeGradient);

}

KisPropertiesConfigurationSP KisGradientMapConfigWidget::configuration() const
{
    KisFilterSP filter = KisFilterRegistry::instance()->get("gradientmap");
    KisFilterConfigurationSP cfg = filter->factoryConfiguration(KisGlobalResourcesInterface::instance());
    if (m_activeGradient) {
        QDomDocument doc;
        QDomElement elt = doc.createElement("gradient");
        m_activeGradient->toXML(doc, elt);
        doc.appendChild(elt);
        cfg->setProperty("gradientXML", doc.toString());
    }

    cfg->setProperty("colorMode", m_ui.colorModeComboBox->currentIndex());
    m_ui.ditherWidget->configuration(*cfg, "dither/");

    return cfg;
}

void KisGradientMapConfigWidget::setConfiguration(const KisPropertiesConfigurationSP config)
{
    Q_ASSERT(config);
    QDomDocument doc;
    if (config->hasProperty("gradientXML")) {
        doc.setContent(config->getString("gradientXML", ""));
        KoStopGradient gradient = KoStopGradient::fromXML(doc.firstChildElement());
        if (gradient.stops().size() > 0) {
            m_activeGradient->setStops(gradient.stops());
            m_activeGradient->updateVariableColors(m_ui.gradientEditor->canvasResourcesInterface());
        }
        m_ui.gradientEditor->setGradient(m_activeGradient);
    }

    m_ui.colorModeComboBox->setCurrentIndex(config->getInt("colorMode"));

    const KisFilterConfiguration *filterConfig = dynamic_cast<const KisFilterConfiguration*>(config.data());
    KIS_SAFE_ASSERT_RECOVER_RETURN(filterConfig);
    m_ui.ditherWidget->setConfiguration(*filterConfig, "dither/");
}

void KisGradientMapConfigWidget::setView(KisViewManager *view)
{
    m_ui.gradientEditor->setCanvasResourcesInterface(view->canvasResourceProvider()->resourceManager()->canvasResourcesInterface());
    m_gradientPopUp->setCanvasResourcesInterface(view->canvasResourceProvider()->resourceManager()->canvasResourcesInterface());
    m_activeGradient->updateVariableColors(m_ui.gradientEditor->canvasResourcesInterface());
}
