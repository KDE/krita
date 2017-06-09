/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
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
    ~DropMergeStrategy() override;
    QString id() const override;
    QString name() const override;
    QString description() const override;
    void merge(Store* dst, QList<const Store*> srcs, QList<double> score) const override;
};
class PriorityToFirstMergeStrategy : public MergeStrategy
{
public:
    PriorityToFirstMergeStrategy();
    ~PriorityToFirstMergeStrategy() override;
    QString id() const override;
    QString name() const override;
    QString description() const override;
    void merge(Store* dst, QList<const Store*> srcs, QList<double> score) const override;
};
class OnlyIdenticalMergeStrategy : public MergeStrategy
{
public:
    OnlyIdenticalMergeStrategy();
    ~OnlyIdenticalMergeStrategy() override;
    QString id() const override;
    QString name() const override;
    QString description() const override;
    void merge(Store* dst, QList<const Store*> srcs, QList<double> score) const override;
};
class SmartMergeStrategy : public MergeStrategy
{
public:
    SmartMergeStrategy();
    ~SmartMergeStrategy() override;
    QString id() const override;
    QString name() const override;
    QString description() const override;
    void merge(Store* dst, QList<const Store*> srcs, QList<double> score) const override;
protected:
    /**
     * Merge multiple entries in one.
     */
    void mergeEntry(Store* dst, QList<const Store*> srcs, const Schema* schema, const QString & identifier) const;
    Value election(QList<const Store*> srcs, QList<double> score, const QString & key) const;
};
}
#endif
