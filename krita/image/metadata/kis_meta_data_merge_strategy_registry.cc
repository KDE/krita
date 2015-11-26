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
#include <QGlobalStatic>

#include "kis_debug.h"
#include "kis_meta_data_merge_strategy_registry.h"
#include "kis_meta_data_merge_strategy_p.h"

Q_GLOBAL_STATIC(KisMetaData::MergeStrategyRegistry, s_instance)


using namespace KisMetaData;

MergeStrategyRegistry::MergeStrategyRegistry()
{
    add(new DropMergeStrategy());
    add(new PriorityToFirstMergeStrategy());
    add(new OnlyIdenticalMergeStrategy());
    add(new SmartMergeStrategy());
}

MergeStrategyRegistry::MergeStrategyRegistry(const MergeStrategyRegistry&) : KoGenericRegistry<const KisMetaData::MergeStrategy*>()
{
}

MergeStrategyRegistry& MergeStrategyRegistry::operator=(const MergeStrategyRegistry&)
{
    return *this;
}

MergeStrategyRegistry::~MergeStrategyRegistry()
{
    Q_FOREACH (const QString &id, keys()) {
        delete get(id);
    }
    dbgRegistry << "Deleting MergeStrategyRegistry";
}

MergeStrategyRegistry* MergeStrategyRegistry::instance()
{
    return s_instance;
}

