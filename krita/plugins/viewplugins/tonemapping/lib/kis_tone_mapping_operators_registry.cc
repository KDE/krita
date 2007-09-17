/*
 *  Copyright (c) 2007 Cyrille Berger (cberger@cberger.net)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_tone_mapping_operators_registry.h"

struct KisToneMappingOperatorsRegistry::Private
{
    static KisToneMappingOperatorsRegistry* s_instance;
};

KisToneMappingOperatorsRegistry* KisToneMappingOperatorsRegistry::Private::s_instance = 0;

KisToneMappingOperatorsRegistry::KisToneMappingOperatorsRegistry() : d(new Private)
{
}

KisToneMappingOperatorsRegistry* KisToneMappingOperatorsRegistry::instance()
{
    if(not Private::s_instance)
    {
        Private::s_instance = new KisToneMappingOperatorsRegistry;
    }
    return Private::s_instance;
}
