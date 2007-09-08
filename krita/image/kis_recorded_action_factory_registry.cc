/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_recorded_action_factory_registry.h"

#include "kis_recorded_paint_actions.h"
#include "kis_recorded_filter_action.h"

#ifdef Q_CC_MSVC
#include <iso646.h>
#endif

struct KisRecordedActionFactoryRegistry::Private {
    static KisRecordedActionFactoryRegistry* instance;
};

KisRecordedActionFactoryRegistry* KisRecordedActionFactoryRegistry::Private::instance = 0;

KisRecordedActionFactoryRegistry* KisRecordedActionFactoryRegistry::instance()
{
    if(not Private::instance)
    {
        Private::instance = new KisRecordedActionFactoryRegistry;
    }
    return Private::instance;
}


KisRecordedActionFactoryRegistry::KisRecordedActionFactoryRegistry() : d(new Private)
{
    add(new KisRecordedFilterActionFactory);
}

KisRecordedActionFactoryRegistry::~KisRecordedActionFactoryRegistry()
{
    delete d;
}
