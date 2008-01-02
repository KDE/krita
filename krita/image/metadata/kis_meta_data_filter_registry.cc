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

#include "kis_meta_data_filter_registry.h"

#include "kis_meta_data_filter_p.h"

using namespace KisMetaData;

FilterRegistry *FilterRegistry::s_singleton = 0;

FilterRegistry::FilterRegistry()
{
    add( new AnonymizerFilter() );
    add( new ToolInfoFilter() );
}

FilterRegistry::FilterRegistry(const FilterRegistry&) : KoGenericRegistry<KisMetaData::Filter*>()
{
}

FilterRegistry& FilterRegistry::operator=(const FilterRegistry& )
{
    return *this;
}

FilterRegistry::~FilterRegistry()
{
}

FilterRegistry* FilterRegistry::instance()
{
    if(not s_singleton)
    {
        s_singleton = new FilterRegistry();
    }
    return s_singleton;
}

