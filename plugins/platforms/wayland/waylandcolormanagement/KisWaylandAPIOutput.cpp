/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisWaylandAPIOutput.h"

#include "KisWaylandAPIImageDescription.h"

KisWaylandAPIOutput::KisWaylandAPIOutput(::wp_color_management_output_v1 *obj)
    : QtWayland::wp_color_management_output_v1(obj)
{
    wp_color_management_output_v1_image_description_changed();
}

KisWaylandAPIOutput::~KisWaylandAPIOutput()
{
    wp_color_management_output_v1_destroy(object());
}

void KisWaylandAPIOutput::wp_color_management_output_v1_image_description_changed()
{
    m_imageDescription = std::make_unique<KisWaylandAPIImageDescription>(get_image_description());
    connect(m_imageDescription.get(), &KisWaylandAPIImageDescription::done, this, &KisWaylandAPIOutput::outputImageDescriptionChanged);
}

#include "moc_KisWaylandAPIOutput.cpp"