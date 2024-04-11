/*
 * SPDX-FileCopyrightText: 2021 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "WGActionManager.h"

#include "WGColorSelectorDock.h"
#include "WGColorPatches.h"
#include "WGColorPreviewToolTip.h"
#include "WGConfigSelectorTypes.h"
#include "WGMyPaintShadeSelector.h"
#include "WGSelectorPopup.h"
#include "WGSelectorWidgetBase.h"
#include "WGShadeSelector.h"

#include <kis_action.h>
#include <kis_action_manager.h>
#include <kis_canvas2.h>
#include <kis_canvas_resource_provider.h>
#include <kis_display_color_converter.h>
#include <kis_signal_compressor.h>
#include <KisViewManager.h>
#include <KisVisualColorSelector.h>

#include <QVector4D>

WGActionManager::WGActionManager(WGColorSelectorDock *parentDock)
    : QObject(parentDock)
    , m_docker(parentDock)
    , m_displayConfig(new WGSelectorDisplayConfig)
    , m_colorTooltip(new WGColorPreviewToolTip)
    , m_colorChangeCompressor(new KisSignalCompressor(100 /* ms */, KisSignalCompressor::POSTPONE, this))
    , m_colorModel(new KisVisualColorModel)
{
    m_lastUsedColor.setOpacity(quint8(0));
    connect(m_colorChangeCompressor, SIGNAL(timeout()), SLOT(slotUpdateDocker()));
    connect(m_colorModel.data(), SIGNAL(sigChannelValuesChanged(QVector4D,quint32)), SLOT(slotChannelValuesChanged()));
    connect(WGConfig::notifier(), SIGNAL(configChanged()), SLOT(slotConfigChanged()));
    connect(WGConfig::notifier(), SIGNAL(selectorConfigChanged()), SLOT(slotSelectorConfigChanged()));
    slotConfigChanged();
}

WGActionManager::~WGActionManager()
{
    delete m_colorSelectorPopup;
    delete m_shadeSelectorPopup;
    delete m_myPaintSelectorPopup;
    delete m_colorHistoryPopup;
}

void WGActionManager::setCanvas(KisCanvas2 *canvas, KisCanvas2 *oldCanvas)
{
    Q_UNUSED(oldCanvas);
    KisDisplayColorConverter *converter = canvas ? canvas->displayColorConverter() : 0;
    m_displayConfig->setDisplayConverter(converter);
    if (m_colorSelector) {
        m_colorSelector->setDisplayRenderer(m_displayConfig->displayConverter()->displayRendererInterface());
    }
}

void WGActionManager::registerActions(KisViewManager *viewManager)
{
    KisActionManager *actionManager = viewManager->actionManager();
    KisAction *action;
    action = actionManager->createAction("show_wg_color_selector");
    connect(action, SIGNAL(triggered()), SLOT(slotShowColorSelectorPopup()));
    action = actionManager->createAction("show_wg_shade_selector");
    connect(action, SIGNAL(triggered()), SLOT(slotShowShadeSelectorPopup()));
    action = actionManager->createAction("show_wg_mypaint_selector");
    connect(action, SIGNAL(triggered()), SLOT(slotShowMyPaintSelectorPopup()));
    action = actionManager->createAction("show_wg_color_history");
    connect(action, SIGNAL(triggered()), SLOT(slotShowColorHistoryPopup()));
    action = actionManager->createAction("wgcs_lighten_color");
    connect(action, SIGNAL(triggered(bool)), SLOT(slotIncreaseLightness()));
    action = actionManager->createAction("wgcs_darken_color");
    connect(action, SIGNAL(triggered(bool)), SLOT(slotDecreaseLightness()));
    action = actionManager->createAction("wgcs_increase_saturation");
    connect(action, SIGNAL(triggered(bool)), SLOT(slotIncreaseSaturation()));
    action = actionManager->createAction("wgcs_decrease_saturation");
    connect(action, SIGNAL(triggered(bool)), SLOT(slotDecreaseSaturation()));
    action = actionManager->createAction("wgcs_shift_hue_clockwise");
    connect(action, SIGNAL(triggered(bool)), SLOT(slotShiftHueCW()));
    action = actionManager->createAction("wgcs_shift_hue_counterclockwise");
    connect(action, SIGNAL(triggered(bool)), SLOT(slotShiftHueCCW()));
}

void WGActionManager::setLastUsedColor(const KoColor &col)
{
    m_lastUsedColor = col;
}

void WGActionManager::updateWidgetSize(QWidget *widget, int size)
{
    QSizePolicy hsp = widget->sizePolicy();
    if (hsp.horizontalPolicy() != QSizePolicy::Fixed) {
        widget->setFixedWidth(size);
    } else {
        widget->setFixedWidth(QWIDGETSIZE_MAX); // looks weird, but that really resets min size to 0...
    }
    if (hsp.verticalPolicy() != QSizePolicy::Fixed) {
        widget->setFixedHeight(size);
    } else {
        widget->setFixedHeight(QWIDGETSIZE_MAX);
    }
}

void WGActionManager::showPopup(WGSelectorPopup *popup)
{
    // preparations
    m_isSynchronizing = true;
    if (m_currentPopup) {
        m_currentPopup->hide();
        m_currentPopup = 0;
    }
    const KisVisualColorModel &dockerModel = m_docker->colorModel();
    m_colorModel->copyState(dockerModel);
    m_colorTooltip->setLastUsedColor(m_displayConfig->displayConverter()->toQColor(m_lastUsedColor));
    QColor baseCol = m_displayConfig->displayConverter()->toQColor(m_colorModel->currentColor());
    m_colorTooltip->setCurrentColor(baseCol);
    m_colorTooltip->setPreviousColor(baseCol);
    m_isSynchronizing = false;

    m_currentPopup = popup;
    popup->slotShowPopup();
    m_colorTooltip->show(popup);
}

void WGActionManager::loadColorSelectorSettings(WGConfig::Accessor &cfg)
{
    m_colorSelector->setRenderMode(cfg.get(WGConfig::selectorRenderMode));
    slotSelectorConfigChanged();
}

void WGActionManager::modifyHSX(int channel, float amount)
{
    if (channel < 0 || channel > 2) {
        return;
    }
    if (m_docker->colorModel().isHSXModel()) {
        QVector4D channelValues = m_docker->colorModel().channelValues();
        channelValues[channel] = qBound(0.0f, channelValues[channel] + amount, 1.0f);
        m_docker->setChannelValues(channelValues);
    }
}

void WGActionManager::slotConfigChanged()
{
    WGConfig::Accessor cfg;
    int popupSize = cfg.get(WGConfig::popupSize);
    bool proofColors = cfg.get(WGConfig::proofToPaintingColors);
    m_displayConfig->setPreviewInPaintingCS(proofColors);

    if (m_colorSelector) {
        loadColorSelectorSettings(cfg);
        m_colorSelector->setProofColors(proofColors);
        updateWidgetSize(m_colorSelector, popupSize);
    }
    if (m_shadeSelector) {
        m_shadeSelector->updateSettings();
        updateWidgetSize(m_shadeSelector, popupSize);
    }
    if (m_myPaintSelector) {
        m_myPaintSelector->updateSettings();
        updateWidgetSize(m_myPaintSelector, popupSize);
    }
    if (m_colorHistoryPopup) {
        WGSelectorWidgetBase *selector = m_colorHistoryPopup->selectorWidget();
        KIS_ASSERT(selector);
        selector->updateSettings();
        updateWidgetSize(selector, popupSize);
    }
}

void WGActionManager::slotSelectorConfigChanged()
{
    if (m_colorSelector) {
        WGConfig::Accessor cfg;
        KisColorSelectorConfiguration selectorConf = cfg.colorSelectorConfiguration();
        m_colorSelector->setConfiguration(&selectorConf);
    }
}

void WGActionManager::slotPopupClosed(WGSelectorPopup *popup)
{
    if (popup == m_currentPopup) {
        m_currentPopup = 0;
        m_colorTooltip->hide();
    }
}

void WGActionManager::slotShowColorSelectorPopup()
{
    if (!m_colorSelectorPopup) {
        WGConfig::Accessor cfg;
        m_colorSelectorPopup = new WGSelectorPopup();
        m_colorSelector = new KisVisualColorSelector(m_colorSelectorPopup, m_colorModel);
        m_colorSelector->setDisplayRenderer(m_displayConfig->displayConverter()->displayRendererInterface());
        updateWidgetSize(m_colorSelector, cfg.get(WGConfig::popupSize));
        m_colorSelectorPopup->setSelectorWidget(m_colorSelector);
        connect(m_colorSelectorPopup, SIGNAL(sigPopupClosed(WGSelectorPopup*)),
                SLOT(slotPopupClosed(WGSelectorPopup*)));
        connect(m_colorSelector, SIGNAL(sigInteraction(bool)), SLOT(slotColorInteraction(bool)));

        loadColorSelectorSettings(cfg);
    }

    // update gamut mask
    KisCanvas2 *canvas = qobject_cast<KisCanvas2*>(m_docker->observedCanvas());
    if (canvas) {
        KisCanvasResourceProvider *resourceProvider = canvas->imageView()->resourceProvider();
        if (resourceProvider->gamutMaskActive()) {
            m_colorSelector->slotGamutMaskChanged(resourceProvider->currentGamutMask());
        }
        else {
            m_colorSelector->slotGamutMaskUnset();
        }
    }

    showPopup(m_colorSelectorPopup);
}

void WGActionManager::slotShowShadeSelectorPopup()
{
    if (!m_shadeSelectorPopup) {
        m_shadeSelectorPopup = new WGSelectorPopup();
        m_shadeSelector = new WGShadeSelector(m_displayConfig, m_colorModel, m_shadeSelectorPopup);
        m_shadeSelector->updateSettings();
        updateWidgetSize(m_shadeSelector, WGConfig::Accessor().get(WGConfig::popupSize));
        m_shadeSelectorPopup->setSelectorWidget(m_shadeSelector);
        connect(m_shadeSelectorPopup, SIGNAL(sigPopupClosed(WGSelectorPopup*)),
                SLOT(slotPopupClosed(WGSelectorPopup*)));
        connect(m_shadeSelector, SIGNAL(sigColorInteraction(bool)), SLOT(slotColorInteraction(bool)));
    }

    showPopup(m_shadeSelectorPopup);
}

void WGActionManager::slotShowMyPaintSelectorPopup()
{
    if (!m_myPaintSelectorPopup) {
        m_myPaintSelectorPopup = new WGSelectorPopup();
        m_myPaintSelector = new WGMyPaintShadeSelector(m_displayConfig, m_myPaintSelectorPopup,
                                                       WGSelectorWidgetBase::PopupMode);
        updateWidgetSize(m_myPaintSelector, WGConfig::Accessor().get(WGConfig::popupSize));
        m_myPaintSelector->setModel(m_colorModel);
        m_myPaintSelectorPopup->setSelectorWidget(m_myPaintSelector);
        connect(m_myPaintSelectorPopup, SIGNAL(sigPopupClosed(WGSelectorPopup*)),
                SLOT(slotPopupClosed(WGSelectorPopup*)));
        connect(m_myPaintSelector, SIGNAL(sigColorInteraction(bool)), SLOT(slotColorInteraction(bool)));
    }

    showPopup(m_myPaintSelectorPopup);
}

void WGActionManager::slotShowColorHistoryPopup()
{
    if (!m_colorHistoryPopup) {
        m_colorHistoryPopup = new WGSelectorPopup;
        WGColorPatches *history = new WGColorPatches(m_displayConfig, m_docker->colorHistory());
        history->setUiMode(WGSelectorWidgetBase::PopupMode);
        history->setPreset(WGColorPatches::History);
        history->updateSettings();
        updateWidgetSize(history, WGConfig::Accessor().get(WGConfig::popupSize));
        m_colorHistoryPopup->setSelectorWidget(history);
        connect(m_colorHistoryPopup, SIGNAL(sigPopupClosed(WGSelectorPopup*)),
                SLOT(slotPopupClosed(WGSelectorPopup*)));
        connect(history, SIGNAL(sigColorInteraction(bool)), SLOT(slotColorPatchInteraction(bool)));
        connect(history, SIGNAL(sigColorChanged(KoColor)), SLOT(slotColorSelected(KoColor)));
    }
    showPopup(m_colorHistoryPopup);
}

void WGActionManager::slotIncreaseLightness()
{
    modifyHSX(2, 0.1f);
}

void WGActionManager::slotDecreaseLightness()
{
    modifyHSX(2, -0.1f);
}

void WGActionManager::slotIncreaseSaturation()
{
    modifyHSX(1, 0.1f);
}

void WGActionManager::slotDecreaseSaturation()
{
    modifyHSX(1, -0.1f);
}

void WGActionManager::slotShiftHueCW()
{
    modifyHSX(0, 0.1f);
}

void WGActionManager::slotShiftHueCCW()
{
    modifyHSX(0, -0.1f);
}

void WGActionManager::slotChannelValuesChanged()
{
    // FIXME: KoColorDisplayRendererInterface's displayConfigurationChanged()
    // signal (e.g. layer switches) makes the color model Q_EMIT new channel values
    // and this would overwrite the color resources with outdated data!
    // so make sure a popup is actually active
    if (!m_isSynchronizing && m_currentPopup) {
        m_colorChangeCompressor->start();
        QColor color = m_displayConfig->displayConverter()->toQColor(m_colorModel->currentColor());
        m_colorTooltip->setCurrentColor(color);
    }
}

void WGActionManager::slotColorInteraction(bool active)
{
    if (active) {
        QColor baseCol = m_displayConfig->displayConverter()->toQColor(m_colorModel->currentColor());
        m_colorTooltip->setCurrentColor(baseCol);
        m_colorTooltip->setPreviousColor(baseCol);
    }
}

void WGActionManager::slotColorPatchInteraction(bool active)
{
    KoCanvasBase *canvas = m_docker->observedCanvas();
    if (active && canvas) {
        KoColor fgColor = canvas->resourceManager()->foregroundColor();
        QColor baseCol = m_docker->displayColorConverter()->toQColor(fgColor);
        m_colorTooltip->setCurrentColor(baseCol);
        m_colorTooltip->setPreviousColor(baseCol);
    }
}

/* Directly set canvas resource for color history,
 */
void WGActionManager::slotColorSelected(const KoColor &color)
{
    // CAVEAT: currently, the color history does not allow background color setting
    if (!m_docker->observedCanvas()) {
        return;
    }
    m_docker->observedCanvas()->resourceManager()->setForegroundColor(color);
    QColor previewCol = m_docker->displayColorConverter()->toQColor(color);
    m_colorTooltip->setCurrentColor(previewCol);

}

void WGActionManager::slotUpdateDocker()
{
    m_docker->setChannelValues(m_colorModel->channelValues());
}
