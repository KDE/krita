/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_transform_mask_params_factory_registry.h"

#include <QGlobalStatic>

#include "kis_transform_mask_params_interface.h"

Q_GLOBAL_STATIC(KisTransformMaskParamsFactoryRegistry, s_instance)


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

void KisTransformMaskParamsFactoryRegistry::setAnimatedParamsHolderFactory(const KisAnimatedTransformMaskParamsHolderFactory &factory)
{
    m_animatedParamsFactory = factory;
}

KisAnimatedTransformParamsHolderInterfaceSP KisTransformMaskParamsFactoryRegistry::createAnimatedParamsHolder(KisDefaultBoundsBaseSP defaultBounds)
{
    KIS_ASSERT(m_animatedParamsFactory);
    return m_animatedParamsFactory(defaultBounds);
}

KisTransformMaskParamsFactoryRegistry*
KisTransformMaskParamsFactoryRegistry::instance()
{
    return s_instance;
}
