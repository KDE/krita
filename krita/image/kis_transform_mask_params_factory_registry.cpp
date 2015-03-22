/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_transform_mask_params_factory_registry.h"

#include <kglobal.h>

#include "kis_transform_mask_params_interface.h"


KisTransformMaskParamsFactoryRegistry::KisTransformMaskParamsFactoryRegistry()
{
}

KisTransformMaskParamsFactoryRegistry::~KisTransformMaskParamsFactoryRegistry()
{
}

void KisTransformMaskParamsFactoryRegistry::addFactory(const QString &id, const KisTransformMaskParamsFactory &factory)
{
    m_map.insert(id, factory);
}

KisTransformMaskParamsInterfaceSP
KisTransformMaskParamsFactoryRegistry::createParams(const QString &id, const QDomElement &e)
{
    KisTransformMaskParamsFactoryMap::iterator it = m_map.find(id);
    return it != m_map.end() ? (*it)(e) : KisTransformMaskParamsInterfaceSP(0);
}

KisTransformMaskParamsFactoryRegistry*
KisTransformMaskParamsFactoryRegistry::instance()
{
    K_GLOBAL_STATIC(KisTransformMaskParamsFactoryRegistry, s_instance);
    return s_instance;
}
