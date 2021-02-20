/*
 *  SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "kis_meta_data_filter_registry.h"
#include "kis_meta_data_filter_p.h"
#include "kis_debug.h"

#include <QGlobalStatic>

using namespace KisMetaData;

Q_GLOBAL_STATIC(FilterRegistry, s_instance)


FilterRegistry::FilterRegistry()
{
    add(new AnonymizerFilter());
    add(new ToolInfoFilter());
}

FilterRegistry::FilterRegistry(const FilterRegistry&)
        : KoGenericRegistry<const KisMetaData::Filter*>()
{
}

FilterRegistry& FilterRegistry::operator=(const FilterRegistry&)
{
    return *this;
}

FilterRegistry::~FilterRegistry()
{
    Q_FOREACH (const QString &id, keys()) {
        delete get(id);
    }
    dbgRegistry << "Deleting FilterRegistry";
    
}

FilterRegistry* FilterRegistry::instance()
{
    return s_instance;
}

