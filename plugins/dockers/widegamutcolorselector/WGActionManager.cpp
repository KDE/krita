/*
 * SPDX-FileCopyrightText: 2021 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "WGActionManager.h"

#include "WGColorSelectorDock.h"
#include "WGSelectorPopup.h"

#include <kactioncollection.h>
#include <kis_action_registry.h>
#include <kis_canvas2.h>
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
    KisActionRegistry *actionRegistry = KisActionRegistry::instance();
    m_colorSelectorPopupAction = actionRegistry->makeQAction("show_wg_color_selector", this);
    connect(m_colorSelectorPopupAction, SIGNAL(triggered()), SLOT(slotShowColorSelectorPopup()));
    m_shadeSelectorPopupAction = actionRegistry->makeQAction("show_wg_shade_selector", this);
    //connect(m_shadeSelectorPopupAction, SIGNAL(triggered()), SLOT(???));
}

void WGActionManager::setCanvas(KisCanvas2 *canvas, KisCanvas2 *oldCanvas)
{
    if (oldCanvas) {
        KisKActionCollection *ac = oldCanvas->viewManager()->actionCollection();
        ac->takeAction(ac->action("show_wg_color_selector"));
        ac->takeAction(ac->action("show_wg_shade_selector"));
    }

    if (canvas) {
        KisKActionCollection* actionCollection = canvas->viewManager()->actionCollection();
        actionCollection->addAction("show_wg_color_selector", m_colorSelectorPopupAction);
        actionCollection->addAction("show_wg_shade_selector", m_shadeSelectorPopupAction);
    }
}

void WGActionManager::slotShowColorSelectorPopup()
{
    if (!m_colorSelectorPopup) {
        m_colorSelectorPopup = new WGSelectorPopup();
        KisVisualColorSelector *selector = new KisVisualColorSelector(m_colorSelectorPopup, m_colorModel);
        selector->setFixedSize(300, 300);
        m_colorSelectorPopup->setSelectorWidget(selector);
    }

    // TODO: make generic sync function
    m_isSynchronizing = true;
    const KisVisualColorModel &dockerModel = m_docker->colorModel();
    m_colorModel->copyState(dockerModel);
    m_isSynchronizing = false;

    m_colorSelectorPopup->slotShowPopup();
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
