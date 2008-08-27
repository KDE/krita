/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_META_DATA_MERGE_STRATEGY_H_
#define _KIS_META_DATA_MERGE_STRATEGY_H_

#include <QList>

#include <krita_export.h>

class QString;

namespace KisMetaData
{
class Store;
/**
 * This is an interface which serves as a base class for meta data store merge strategy.
 */
class KRITAIMAGE_EXPORT MergeStrategy
{
public:
    virtual ~MergeStrategy();
    /// @return the id of this merge strategy
    virtual QString id() const = 0;
    /// @return the name of this merge strategy
    virtual QString name() const = 0;
    /// @return a description of this merge strategy
    virtual QString description() const = 0;
    /**
     * Call this function to merge a list of meta data stores in one.
     * @param dst the destination store
     * @param srcs the list of source meta data store
     */
    virtual void merge(Store* dst, QList<const Store*> srcs, QList<double> score) const = 0;
};

}

#endif
