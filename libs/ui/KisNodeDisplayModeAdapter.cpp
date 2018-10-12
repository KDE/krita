/*
 *  Copyright (c) 2018 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
