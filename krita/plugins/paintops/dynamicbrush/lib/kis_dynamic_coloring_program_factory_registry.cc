/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
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

#include "kis_dynamic_coloring_program_factory_registry.h"

struct KisDynamicColoringProgramFactoryRegistry::Private {
    static KisDynamicColoringProgramFactoryRegistry* instance;
};

KisDynamicColoringProgramFactoryRegistry* KisDynamicColoringProgramFactoryRegistry::Private::instance = 0;

KisDynamicColoringProgramFactoryRegistry::KisDynamicColoringProgramFactoryRegistry() : d(new Private)
{
}

KisDynamicColoringProgramFactoryRegistry* KisDynamicColoringProgramFactoryRegistry::instance()
{
    if(Private::instance ==0)
    {
        Private::instance = new KisDynamicColoringProgramFactoryRegistry;
    }
    return Private::instance;
}

KisDynamicProgramFactory* KisDynamicColoringProgramFactoryRegistry::programFactory(QString id) const
{
    return get(id);
}

QList<KoID> KisDynamicColoringProgramFactoryRegistry::programTypes() const
{
    return listKeys();
}

