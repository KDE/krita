/*
 *  SPDX-FileCopyrightText: 2018 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisNodeDisplayModeAdapter.h"
#include "kis_config.h"
#include "kis_config_notifier.h"


KisNodeDisplayModeAdapter::KisNodeDisplayModeAdapter(QObject *parent)
    : QObject(parent)
{
    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()),
            SLOT(slotSettingsChanged()));

    slotSettingsChangedImpl(true);
}

bool KisNodeDisplayModeAdapter::showRootNode() const
{
    return m_showRootNode;
}

void KisNodeDisplayModeAdapter::setShowRootNode(bool value)
{
    KisConfig cfg(false);
    cfg.setShowRootLayer(value);
    slotSettingsChanged();
}

bool KisNodeDisplayModeAdapter::showGlobalSelectionMask() const
{
    return m_showGlobalSelectionMask;
}

void KisNodeDisplayModeAdapter::setShowGlobalSelectionMask(bool value)
{
    KisConfig cfg(false);
    cfg.setShowGlobalSelection(value);
    slotSettingsChanged();
}

void KisNodeDisplayModeAdapter::slotSettingsChanged()
{
    slotSettingsChangedImpl(false);
}

void KisNodeDisplayModeAdapter::slotSettingsChangedImpl(bool suppressSignals)
{
    KisConfig cfg(true);

    if (m_showGlobalSelectionMask != cfg.showGlobalSelection() ||
        m_showRootNode != cfg.showRootLayer()) {

        m_showGlobalSelectionMask = cfg.showGlobalSelection();
        m_showRootNode = cfg.showRootLayer();

        if (!suppressSignals) {
            emit sigNodeDisplayModeChanged(m_showRootNode, m_showGlobalSelectionMask);
        }
    }
}
