/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_NODE_FILTER_INTERFACE_H_
#define _KIS_NODE_FILTER_INTERFACE_H_

#include <krita_export.h>
#include <kis_types.h>
#include "filter/kis_filter_configuration.h"

/**
 * Define an interface for nodes that are associated with a filter.
 */
class KRITAIMAGE_EXPORT KisNodeFilterInterface
{
public:
    KisNodeFilterInterface(KisFilterConfiguration *filterConfig, bool useGeneratorRegistry);
    KisNodeFilterInterface(const KisNodeFilterInterface &rhs);
    virtual ~KisNodeFilterInterface();

    /**
     * @return safe shared pointer to the filter configuration
     *         associated with this node
     */
    virtual KisSafeFilterConfigurationSP filter() const;

    /**
     * Sets the filter configuration for this node. The filter might
     * differ from the filter that is currently set up on this node.
     *
     * WARNING: the filterConfig becomes *owned* by the node right
     * after you've set it. Don't try to access the configuration
     * after you've associated it with the node.
     */
    virtual void setFilter(KisFilterConfiguration *filterConfig);

// the child classes should access the filter with the filter() method
private:

    KisSafeFilterConfigurationSP m_filter;
    bool m_useGeneratorRegistry;
};

#endif
