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

#ifdef SANITY_CHECK_FILTER_CONFIGURATION_OWNER

#define SANITY_ACQUIRE_FILTER(filter)                           \
    do {                                                        \
        if ((filter)) {                                         \
            (filter)->sanityRefUsageCounter();                  \
        }                                                       \
    } while (0)

#define SANITY_RELEASE_FILTER(filter)                           \
    do {                                                        \
        if (m_filter && m_filter->sanityDerefUsageCounter()) {  \
            warnKrita;                                                 \
            warnKrita << "WARNING: filter configuration has more than one user! Krita will probably crash soon!"; \
            warnKrita << "WARNING:" << ppVar(this);                    \
            warnKrita << "WARNING:" << ppVar(filter.data());           \
            warnKrita;                                                 \
        }                                                               \
    } while (0)

#else /* SANITY_CHECK_FILTER_CONFIGURATION_OWNER */

#define SANITY_ACQUIRE_FILTER(filter)
#define SANITY_RELEASE_FILTER(filter)

#endif /* SANITY_CHECK_FILTER_CONFIGURATION_OWNER*/

KisNodeFilterInterface::KisNodeFilterInterface(KisFilterConfigurationSP filterConfig, bool useGeneratorRegistry)
    : m_filter(filterConfig),
      m_useGeneratorRegistry(useGeneratorRegistry)
{
    SANITY_ACQUIRE_FILTER(m_filter);
}

KisNodeFilterInterface::KisNodeFilterInterface(const KisNodeFilterInterface &rhs)
    : m_useGeneratorRegistry(rhs.m_useGeneratorRegistry)
{
    if (m_useGeneratorRegistry) {
        m_filter = KisGeneratorRegistry::instance()->cloneConfiguration(const_cast<KisFilterConfiguration*>(rhs.m_filter.data()));
    } else {
        m_filter = KisFilterRegistry::instance()->cloneConfiguration(const_cast<KisFilterConfiguration*>(rhs.m_filter.data()));
    }

    SANITY_ACQUIRE_FILTER(m_filter);
}

KisNodeFilterInterface::~KisNodeFilterInterface()
{
    SANITY_RELEASE_FILTER(m_filter);
}

KisFilterConfigurationSP KisNodeFilterInterface::filter() const
{
    return m_filter;
}

void KisNodeFilterInterface::setFilter(KisFilterConfigurationSP filterConfig)
{
    SANITY_RELEASE_FILTER(m_filter);

    KIS_SAFE_ASSERT_RECOVER_RETURN(filterConfig);
    m_filter = filterConfig;

    SANITY_ACQUIRE_FILTER(m_filter);
}
