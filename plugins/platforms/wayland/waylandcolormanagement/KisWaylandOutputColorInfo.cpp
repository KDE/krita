/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisWaylandOutputColorInfo.h"

#include <QScreen>
#include <QTimer>
#include <QPointer>

#include <qpa/qplatformnativeinterface.h>

#include "KisWaylandAPIColorManager.h"
#include "KisWaylandAPIImageDescription.h"
#include "KisWaylandAPIOutput.h"

#include "KisWaylandSurfaceColorManager.h"

 #include <QtAssert>
 #include <kis_debug.h>

KisWaylandOutputColorInfo::KisWaylandOutputColorInfo(QObject *parent)
    : KisOutputColorInfoInterface(parent)
{
    m_waylandManager = KisWaylandSurfaceColorManager::getOrCreateGlobalWaylandManager();
    connect(m_waylandManager.get(), &KisWaylandAPIColorManager::sigReadyChanged,
            this, &KisWaylandOutputColorInfo::reinitialize);
    reinitialize();

    connect(qApp, &QGuiApplication::screenAdded, this, &KisWaylandOutputColorInfo::slotScreenAdded);
    connect(qApp, &QGuiApplication::screenRemoved, this, &KisWaylandOutputColorInfo::slotScreenRemoved);
}

KisWaylandOutputColorInfo::~KisWaylandOutputColorInfo()
{
}

void KisWaylandOutputColorInfo::setReadyImpl(bool value)
{
    if (value == m_isReady) return;

    m_isReady = value;
    Q_EMIT sigReadyChanged(m_isReady);
}

bool KisWaylandOutputColorInfo::checkIfAllReady() const
{
    if (m_waylandOutputs.empty()) return false;

    return std::find_if(m_waylandOutputs.begin(), m_waylandOutputs.end(),
        [] (const auto &it) {
            return !it.second->m_imageDescription->info.isReady();
        }
        ) == m_waylandOutputs.end();
}

void KisWaylandOutputColorInfo::reinitialize()
{
    m_waylandOutputs.clear();

    if (!m_waylandManager->isReady()) {
        setReadyImpl(false);
        return;
    }

    for (QScreen *screen : qApp->screens()) {
        initScreenConnection(screen);
    }

    setReadyImpl(checkIfAllReady());
}

void KisWaylandOutputColorInfo::initScreenConnection(QScreen *screen)
{
    auto waylandScreen = screen->nativeInterface<QNativeInterface::QWaylandScreen>();

    // the screen may have no wayland screen if it is a fake placeholder screen
    // that was created by Qt while compositor restart
    if (!waylandScreen) return;

    ::wl_output *output = waylandScreen->output();
    ::wp_color_management_output_v1 *cmoutput = m_waylandManager->get_output(output);
    Q_ASSERT(cmoutput);

    auto outputObject = std::make_unique<KisWaylandAPIOutput>(cmoutput);
    connect(outputObject.get(), &KisWaylandAPIOutput::outputImageDescriptionChanged, outputObject.get(), [this, screen, outputPtr = outputObject.get()]() {
        auto desc = outputPtr->m_imageDescription->info.m_data;
        Q_EMIT sigOutputDescriptionChanged(screen, desc.toSurfaceDescription());

        setReadyImpl(checkIfAllReady());
    });

    m_waylandOutputs.insert_or_assign(screen, std::move(outputObject));
}

void KisWaylandOutputColorInfo::slotScreenAdded(QScreen *screen)
{
    if (!m_waylandManager->isActive()) {
        return;
    }

    initScreenConnection(screen);

    setReadyImpl(checkIfAllReady());
}

void KisWaylandOutputColorInfo::slotScreenRemoved(QScreen *screen)
{
    if (!m_waylandManager->isActive()) {
        return;
    }

    m_waylandOutputs.erase(screen);

    setReadyImpl(checkIfAllReady());
}

bool KisWaylandOutputColorInfo::isReady() const
{
    return m_isReady;
}

std::optional<KisSurfaceColorimetry::SurfaceDescription> KisWaylandOutputColorInfo::outputDescription(const QScreen *screen) const
{
    auto it = m_waylandOutputs.find(screen);
    if (it == m_waylandOutputs.end()) return std::nullopt;

    const auto &info = it->second->m_imageDescription->info;

    if (!info.isReady()) {
        return std::nullopt;
    }

    return info.m_data.toSurfaceDescription();
}

#include <moc_KisWaylandOutputColorInfo.cpp>