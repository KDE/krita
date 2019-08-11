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
