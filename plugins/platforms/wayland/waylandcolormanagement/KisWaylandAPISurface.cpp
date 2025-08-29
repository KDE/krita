/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisWaylandAPISurface.h"

#include "KisWaylandAPIImageDescription.h"


KisWaylandAPISurfaceFeedback::KisWaylandAPISurfaceFeedback(::wp_color_management_surface_feedback_v1 *obj)
    : QtWayland::wp_color_management_surface_feedback_v1(obj)
    , m_preferred(std::make_unique<KisWaylandAPIImageDescription>(get_preferred()))
{
    connect(m_preferred.get(), &KisWaylandAPIImageDescription::done, this, &KisWaylandAPISurfaceFeedback::preferredChanged);
}

KisWaylandAPISurfaceFeedback::~KisWaylandAPISurfaceFeedback()
{
    wp_color_management_surface_feedback_v1_destroy(object());
}

void KisWaylandAPISurfaceFeedback::wp_color_management_surface_feedback_v1_preferred_changed(uint32_t identity)
{
    Q_UNUSED(identity);
    m_preferred = std::make_unique<KisWaylandAPIImageDescription>(get_preferred());
    connect(m_preferred.get(), &KisWaylandAPIImageDescription::done, this, &KisWaylandAPISurfaceFeedback::preferredChanged);
}

KisWaylandAPISurface::KisWaylandAPISurface(::wp_color_management_surface_v1 *obj, std::unique_ptr<KisWaylandAPISurfaceFeedback> &&feedback)
    : QtWayland::wp_color_management_surface_v1(obj)
    , m_feedback(std::move(feedback))
{
}

KisWaylandAPISurface::~KisWaylandAPISurface()
{
    wp_color_management_surface_v1_destroy(object());
}

#include "moc_KisWaylandAPISurface.cpp"