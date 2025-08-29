/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisWaylandAPIColorManager.h"

#include <QDebug>

KisWaylandAPIColorManager::KisWaylandAPIColorManager()
    : QWaylandClientExtensionTemplate(1)
{
    connect(this, &QWaylandClientExtensionTemplate::activeChanged, this, [this] {
        if (!isActive()) {
            m_supportedFeatures.clear();
            m_supportedIntents.clear();
            m_supportedPrimariesNamed.clear();
            m_supportedTransferFunctionsNamed.clear();
            if (m_isReady) {
                m_isReady = false;
                Q_EMIT sigReadyChanged(m_isReady);
            }
        }
    });
}

KisWaylandAPIColorManager::~KisWaylandAPIColorManager()
{
    // the destruction of the member object is automatically handled
    // be the extension template class (thanks to the second template
    // parameter)
}

void KisWaylandAPIColorManager::wp_color_manager_v1_supported_intent(uint32_t _render_intent)
{
    m_supportedIntents.insert(static_cast<render_intent>(_render_intent));
}
void KisWaylandAPIColorManager::wp_color_manager_v1_supported_feature(uint32_t _feature)
{
    m_supportedFeatures.insert(static_cast<feature>(_feature));
}
void KisWaylandAPIColorManager::wp_color_manager_v1_supported_tf_named(uint32_t tf)
{
    m_supportedTransferFunctionsNamed.insert(static_cast<transfer_function>(tf));
}
void KisWaylandAPIColorManager::wp_color_manager_v1_supported_primaries_named(uint32_t _primaries)
{
    m_supportedPrimariesNamed.insert(static_cast<primaries>(_primaries));
}
void KisWaylandAPIColorManager::wp_color_manager_v1_done()
{
    if (m_isReady) {
        qWarning() << "WARNING: KisWaylandAPIColorManager::wp_color_manager_v1_done(): done event arrived while ready is true";
    }

    m_isReady = true;
    Q_EMIT sigReadyChanged(m_isReady);
}

#include "moc_KisWaylandAPIColorManager.cpp"