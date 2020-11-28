/*
 *  SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
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
