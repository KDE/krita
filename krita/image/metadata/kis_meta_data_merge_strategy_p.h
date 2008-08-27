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
#ifndef KIS_META_DATA_MERGE_STRATEGY_P_H
#define KIS_META_DATA_MERGE_STRATEGY_P_H

#include "kis_meta_data_merge_strategy.h"

class QString;

namespace KisMetaData
{
class Schema;
class Value;
/**
 * This strategy drop all meta data.
 */
class DropMergeStrategy : public MergeStrategy
{
public:
    DropMergeStrategy();
    virtual ~DropMergeStrategy();
    virtual QString id() const;
    virtual QString name() const;
    virtual QString description() const;
    virtual void merge(Store* dst, QList<const Store*> srcs, QList<double> score) const;
};
class PriorityToFirstMergeStrategy : public MergeStrategy
{
public:
    PriorityToFirstMergeStrategy();
    virtual ~PriorityToFirstMergeStrategy();
    virtual QString id() const;
    virtual QString name() const;
    virtual QString description() const;
    virtual void merge(Store* dst, QList<const Store*> srcs, QList<double> score) const;
};
class OnlyIdenticalMergeStrategy : public MergeStrategy
{
public:
    OnlyIdenticalMergeStrategy();
    virtual ~OnlyIdenticalMergeStrategy();
    virtual QString id() const;
    virtual QString name() const;
    virtual QString description() const;
    virtual void merge(Store* dst, QList<const Store*> srcs, QList<double> score) const;
};
class SmartMergeStrategy : public MergeStrategy
{
public:
    SmartMergeStrategy();
    virtual ~SmartMergeStrategy();
    virtual QString id() const;
    virtual QString name() const;
    virtual QString description() const;
    virtual void merge(Store* dst, QList<const Store*> srcs, QList<double> score) const;
protected:
    /**
     * Merge multiple entries in one.
     */
    void mergeEntry(Store* dst, QList<const Store*> srcs, const Schema* schema, const QString & identifier) const;
    Value election(QList<const Store*> srcs, QList<double> score, const QString & key) const;
};
}
#endif
