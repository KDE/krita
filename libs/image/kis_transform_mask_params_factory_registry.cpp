/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_transform_mask_params_factory_registry.h"

#include <QGlobalStatic>

#include "kis_transform_mask_params_interface.h"
#include "kis_transform_mask.h"

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

void KisTransformMaskParamsFactoryRegistry::setAnimatedParamsFactory(const KisAnimatedTransformMaskParamsFactory &factory)
{
    m_animatedParamsFactory = factory;
}

KisTransformMaskParamsInterfaceSP KisTransformMaskParamsFactoryRegistry::animateParams(KisTransformMaskParamsInterfaceSP params, const KisTransformMaskSP mask)
{
    if (!m_animatedParamsFactory) return KisTransformMaskParamsInterfaceSP();

    return m_animatedParamsFactory(params, mask);
}

void KisTransformMaskParamsFactoryRegistry::setKeyframeFactory(const KisTransformMaskKeyframeFactory &factory)
{
    m_keyframeFactory = factory;
}

void KisTransformMaskParamsFactoryRegistry::autoAddKeyframe(KisTransformMaskSP mask, int time, KisTransformMaskParamsInterfaceSP params, KUndo2Command *parentCommand)
{
    if (m_keyframeFactory) {
        m_keyframeFactory(mask, time, params, parentCommand);
    }
}

KisTransformMaskParamsFactoryRegistry*
KisTransformMaskParamsFactoryRegistry::instance()
{
    return s_instance;
}
