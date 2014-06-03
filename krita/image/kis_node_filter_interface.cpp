/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2012 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_node_filter_interface.h"

#include "filter/kis_filter.h"
#include "generator/kis_generator.h"
#include "filter/kis_filter_registry.h"
#include "filter/kis_filter_configuration.h"
#include "generator/kis_generator_registry.h"

KisNodeFilterInterface::KisNodeFilterInterface(KisFilterConfiguration *filterConfig, bool useGeneratorRegistry)
    : m_filter(filterConfig),
      m_useGeneratorRegistry(useGeneratorRegistry)
{
}

KisNodeFilterInterface::KisNodeFilterInterface(const KisNodeFilterInterface &rhs)
    : m_useGeneratorRegistry(rhs.m_useGeneratorRegistry)
{
    if (m_useGeneratorRegistry) {
        m_filter = KisSafeFilterConfigurationSP(KisGeneratorRegistry::instance()->cloneConfiguration(rhs.m_filter.data()));
    } else {
        m_filter = KisSafeFilterConfigurationSP(KisFilterRegistry::instance()->cloneConfiguration(rhs.m_filter.data()));
    }
}

KisNodeFilterInterface::~KisNodeFilterInterface()
{
}

KisSafeFilterConfigurationSP KisNodeFilterInterface::filter() const
{
    return m_filter;
}

void KisNodeFilterInterface::setFilter(KisFilterConfiguration *filterConfig)
{
    Q_ASSERT(filterConfig);
    m_filter = KisSafeFilterConfigurationSP(filterConfig);
}
