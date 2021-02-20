/*
 *  SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef KIS_META_DATA_FILTER_P_H
#define KIS_META_DATA_FILTER_P_H

#include "kis_meta_data_filter.h"

namespace KisMetaData
{
/**
 * Filter that remove personal data in a meta store.
 */
class AnonymizerFilter : public Filter
{
public:
    ~AnonymizerFilter() override;
    QString id() const override;
    QString name() const override;
    QString description() const override;
    bool defaultEnabled() const override;
    void filter(KisMetaData::Store*) const override;
};
/**
 * Filter that add the name of the creation program and the date
 * of the last modification.
 */
class ToolInfoFilter : public Filter
{
public:
    ~ToolInfoFilter() override;
    QString id() const override;
    QString name() const override;
    QString description() const override;
    bool defaultEnabled() const override;
    void filter(KisMetaData::Store*) const override;
};
}
#endif
