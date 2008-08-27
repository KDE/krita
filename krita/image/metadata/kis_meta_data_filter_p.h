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
    virtual ~AnonymizerFilter();
    virtual QString id() const;
    virtual QString name() const;
    virtual QString description() const;
    virtual bool defaultEnabled() const;
    virtual void filter(KisMetaData::Store*) const;
};
/**
 * Filter that add the name of the creation program and the date
 * of the last modificiation.
 */
class ToolInfoFilter : public Filter
{
public:
    virtual ~ToolInfoFilter();
    virtual QString id() const;
    virtual QString name() const;
    virtual QString description() const;
    virtual bool defaultEnabled() const;
    virtual void filter(KisMetaData::Store*) const;
};
}
#endif
