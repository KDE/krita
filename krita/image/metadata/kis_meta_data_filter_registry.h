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

#ifndef _KIS_META_DATA_FILTER_REGISTRY_H_
#define _KIS_META_DATA_FILTER_REGISTRY_H_

#include <krita_export.h>

#include "KoGenericRegistry.h"
#include "kis_meta_data_filter.h"

namespace KisMetaData
{

class KRITAIMAGE_EXPORT FilterRegistry : public KoGenericRegistry<const KisMetaData::Filter*>
{
public:
    virtual ~FilterRegistry();
    static FilterRegistry* instance();
private:
    FilterRegistry();
    FilterRegistry(const FilterRegistry&);
    FilterRegistry& operator=(const FilterRegistry&);
};

}

#endif
