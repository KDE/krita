/*
 *  SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
        if (m_filterConfiguration && m_filterConfiguration->sanityDerefUsageCounter()) {  \
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

KisNodeFilterInterface::KisNodeFilterInterface(KisFilterConfigurationSP filterConfig)
    : m_filterConfiguration(filterConfig)
{
    SANITY_ACQUIRE_FILTER(m_filterConfiguration);
    KIS_SAFE_ASSERT_RECOVER_NOOP(!filterConfig || filterConfig->hasLocalResourcesSnapshot());
}

KisNodeFilterInterface::KisNodeFilterInterface(const KisNodeFilterInterface &rhs)
    : m_filterConfiguration(rhs.m_filterConfiguration->clone())

{
    SANITY_ACQUIRE_FILTER(m_filterConfiguration);
}

KisNodeFilterInterface::~KisNodeFilterInterface()
{
    SANITY_RELEASE_FILTER(m_filterConfiguration);
}

KisFilterConfigurationSP KisNodeFilterInterface::filter() const
{
    return m_filterConfiguration;
}

void KisNodeFilterInterface::setFilter(KisFilterConfigurationSP filterConfig)
{
    SANITY_RELEASE_FILTER(m_filterConfiguration);

    KIS_SAFE_ASSERT_RECOVER_RETURN(filterConfig);
    KIS_SAFE_ASSERT_RECOVER_NOOP(filterConfig->hasLocalResourcesSnapshot());
    m_filterConfiguration = filterConfig;

    SANITY_ACQUIRE_FILTER(m_filterConfiguration);
}
