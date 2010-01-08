/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004-2008 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Clarence Dang <dang@kde.org>
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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

#include "kis_filterop_settings.h"
#include "kis_filterop_settings_widget.h"

#include <kis_brush_option.h>
#include <kis_paintop_options_widget.h>
#include <kis_pressure_size_option.h>
#include <kis_filter_option.h>
#include <filter/kis_filter.h>
#include <filter/kis_filter_configuration.h>
#include <kis_node.h>
#include <kis_image.h>


KisFilterOpSettings::KisFilterOpSettings()
        : m_options(0)
{
}

KisFilterOpSettings::~KisFilterOpSettings()
{
}

bool KisFilterOpSettings::paintIncremental()
{
    return true; // We always paint on the existing data
}

void KisFilterOpSettings::setNode(KisNodeSP node)
{
    KisPaintOpSettings::setNode(node);
    if (m_options) {
        m_options->m_filterOption->setNode(node);
    }
}

void KisFilterOpSettings::setImage(KisImageWSP image)
{
    if (m_options) {
        m_options->m_filterOption->setImage(image);
    }
}

KisFilterSP KisFilterOpSettings::filter() const
{
    return m_options->m_filterOption->filter();
}

KisFilterConfiguration* KisFilterOpSettings::filterConfig() const
{
    return m_options->m_filterOption->filterConfig();
}

bool KisFilterOpSettings::ignoreAlpha() const
{
    return m_options->m_filterOption->ignoreAlpha();
}
