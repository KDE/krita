/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_TRANSFORM_MASK_PARAMS_FACTORY_REGISTRY_H
#define __KIS_TRANSFORM_MASK_PARAMS_FACTORY_REGISTRY_H

#include <QMap>

#include <functional>

#include "kis_types.h"
#include "kritaimage_export.h"


class QDomElement;

using KisTransformMaskParamsFactory    = std::function<KisTransformMaskParamsInterfaceSP (const QDomElement &)>;
using KisTransformMaskParamsFactoryMap = QMap<QString, KisTransformMaskParamsFactory>;
using KisAnimatedTransformMaskParamsFactory = std::function<KisTransformMaskParamsInterfaceSP (KisTransformMaskParamsInterfaceSP, const KisTransformMaskSP)>;
using KisTransformMaskKeyframeFactory = std::function<void (KisTransformMaskSP, int, KisTransformMaskParamsInterfaceSP, KUndo2Command*)>;

class KRITAIMAGE_EXPORT KisTransformMaskParamsFactoryRegistry
{

public:
    KisTransformMaskParamsFactoryRegistry();
    ~KisTransformMaskParamsFactoryRegistry();

    void addFactory(const QString &id, const KisTransformMaskParamsFactory &factory);
    KisTransformMaskParamsInterfaceSP createParams(const QString &id, const QDomElement &e);

    void setAnimatedParamsFactory(const KisAnimatedTransformMaskParamsFactory &factory);
    KisTransformMaskParamsInterfaceSP animateParams(KisTransformMaskParamsInterfaceSP params, const KisTransformMaskSP mask);

    void setKeyframeFactory(const KisTransformMaskKeyframeFactory &factory);
    void autoAddKeyframe(KisTransformMaskSP mask, int time, KisTransformMaskParamsInterfaceSP params, KUndo2Command *parentCommand);

    static KisTransformMaskParamsFactoryRegistry* instance();

private:
    KisTransformMaskParamsFactoryMap m_map;
    KisAnimatedTransformMaskParamsFactory m_animatedParamsFactory;
    KisTransformMaskKeyframeFactory m_keyframeFactory;
};

#endif /* __KIS_TRANSFORM_MASK_PARAMS_FACTORY_REGISTRY_H */
