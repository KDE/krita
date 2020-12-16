/*
 *  SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef _KIS_META_DATA_MERGE_STRATEGY_H_
#define _KIS_META_DATA_MERGE_STRATEGY_H_

#include <QList>

#include <kritametadata_export.h>

class QString;

namespace KisMetaData
{
class Store;
/**
 * This is an interface which serves as a base class for meta data store merge
 * strategy.
 * This is used to decide which entries of a metadata store is kept, or how they
 * are modified when a list of meta data stores are merged together.
 */
class KRITAMETADATA_EXPORT MergeStrategy
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
     * @param scores a list of score which defines the importance of each store compared to the other
     *              the sum of score is expected to be equal to 1.0.
     *              One way to attribute a score is to compute the area of each layer and then
     *              to give a higher score to the biggest layer.
     * srcs and scores list must have the same size.
     */
    virtual void merge(Store* dst, QList<const Store*> srcs, QList<double> scores) const = 0;
};

}

#endif
