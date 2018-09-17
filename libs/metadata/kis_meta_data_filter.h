/*
 *  Copyright (c) 2007-2008 Cyrille Berger <cberger@cberger.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_META_DATA_FILTER_H_
#define _KIS_META_DATA_FILTER_H_

#include <kritametadata_export.h>
class QString;

namespace KisMetaData
{
class Store;
/**
 * This class is a base class for filtering a meta data store to alter some
 * information. For instance, remove author information or change edition
 * date.
 */
class Filter
{
public:
    virtual ~Filter();
    /// @return true if the filter is enabled by default when exporting
    virtual bool defaultEnabled() const = 0;
    /// @return the id of this filter
    virtual QString id() const = 0;
    /// @return the name of this filter
    virtual QString name() const = 0;
    /// @return a description of this filter
    virtual QString description() const = 0;
    /**
     * Apply a filter on a meta data store.
     */
    virtual void filter(KisMetaData::Store*) const = 0;
};
}

#endif
