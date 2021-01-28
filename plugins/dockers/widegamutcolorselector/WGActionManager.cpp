/*
 * SPDX-FileCopyrightText: 2021 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "WGActionManager.h"

#include "WGColorSelectorDock.h"
#include "WGSelectorPopup.h"
#include "WGShadeSelector.h"

#include <kis_action.h>
#include <kis_action_manager.h>
#include <kis_canvas2.h>
#include <kis_canvas_resource_provider.h>
#include <kis_signal_compressor.h>
#include <KisViewManager.h>
#include <KisVisualColorSelector.h>

#include <QVector4D>

WGActionManager::WGActionManager(WGColorSelectorDock *parentDock)
    : QObject(parentDock)
    , m_docker(parentDock)
    , m_colorChangeCompressor(new KisSignalCompressor(100 /* ms */, KisSignalCompressor::POSTPONE, this))
    , m_colorModel(new KisVisualColorModel)
{
    connect(m_colorChangeCompressor, SIGNAL(timeout()), SLOT(slotUpdateDocker()));
    connect(m_colorModel.data(), SIGNAL(sigChannelValuesChanged(QVector4D)), SLOT(slotChannelValuesChanged()));
}

/* void WGActionManager::setCanvas(KisCanvas2 *canvas, KisCanvas2 *oldCanvas)
{
} */

void WGActionManager::registerActions(KisViewManager *viewManager)
{
    KisActionManager *actionManager = viewManager->actionManager();
    KisAction *action;
    action = actionManager->createAction("show_wg_color_selector");
    connect(action, SIGNAL(triggered()), SLOT(slotShowColorSelectorPopup()));
    action = actionManager->createAction("show_wg_shade_selector");
    connect(action, SIGNAL(triggered()), SLOT(slotShowShadeSelectorPopup()));
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

void WGActionManager::preparePopup()
{
    m_isSynchronizing = true;
    const KisVisualColorModel &dockerModel = m_docker->colorModel();
    m_colorModel->copyState(dockerModel);
    m_isSynchronizing = false;
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

void WGActionManager::slotShowColorSelectorPopup()
{
    if (!m_colorSelectorPopup) {
        m_colorSelectorPopup = new WGSelectorPopup();
        m_colorSelector = new KisVisualColorSelector(m_colorSelectorPopup, m_colorModel);
        m_colorSelector->setFixedSize(300, 300);
        m_colorSelectorPopup->setSelectorWidget(m_colorSelector);
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

    preparePopup();

    m_colorSelectorPopup->slotShowPopup();
}

void WGActionManager::slotShowShadeSelectorPopup()
{
    if (!m_shadeSelectorPopup) {
        m_shadeSelectorPopup = new WGSelectorPopup();
        m_shadeSelector = new WGShadeSelector(m_colorModel, m_shadeSelectorPopup);
        m_shadeSelector->setFixedWidth(300);
        m_shadeSelectorPopup->setSelectorWidget(m_shadeSelector);
    }
    preparePopup();
    m_shadeSelectorPopup->slotShowPopup();
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
    if (!m_isSynchronizing) {
        m_colorChangeCompressor->start();
    }
}

void WGActionManager::slotUpdateDocker()
{
    m_docker->setChannelValues(m_colorModel->channelValues());
}
