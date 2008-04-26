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

class KisFilterConfiguration;

/**
 * Define an interface for nodes that are associated with a filter.
 */
class KisNodeFilterInterface {
  public:
    virtual ~KisNodeFilterInterface() {}
    /**
     * @return the filter configuration associated with this node
     */
    virtual KisFilterConfiguration * filter() const = 0;
    /**
     * Set the new filter configuration (this can be a different filter).
     */
    virtual void setFilter(KisFilterConfiguration * filterConfig) = 0;
};

#endif
