/*
 *  SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_NODE_FILTER_INTERFACE_H_
#define _KIS_NODE_FILTER_INTERFACE_H_

#include <kritaimage_export.h>
#include <kis_types.h>

/**
 * Define an interface for nodes that are associated with a filter.
 */
class KRITAIMAGE_EXPORT KisNodeFilterInterface
{
public:
    KisNodeFilterInterface(KisFilterConfigurationSP filterConfig);
    KisNodeFilterInterface(const KisNodeFilterInterface &rhs);
    virtual ~KisNodeFilterInterface();

    /**
     * @return safe shared pointer to the filter configuration
     *         associated with this node
     */
    virtual KisFilterConfigurationSP filter() const;

    /**
     * Sets the filter configuration for this node. The filter might
     * differ from the filter that is currently set up on this node.
     *
     * WARNING: the filterConfig becomes *owned* by the node right
     * after you've set it. Don't try to access the configuration
     * after you've associated it with the node.
     */
    virtual void setFilter(KisFilterConfigurationSP filterConfig);

// the child classes should access the filter with the filter() method
private:
    KisNodeFilterInterface& operator=(const KisNodeFilterInterface &other);

    KisFilterConfigurationSP m_filterConfiguration;
};

#endif
