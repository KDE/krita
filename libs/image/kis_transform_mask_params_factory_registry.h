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

#include "kis_transform_mask_params_interface.h"


class QDomElement;

using KisTransformMaskParamsFactory    = std::function<KisTransformMaskParamsInterfaceSP (const QDomElement &)>;
using KisTransformMaskParamsFactoryMap = QMap<QString, KisTransformMaskParamsFactory>;
using KisAnimatedTransformMaskParamsHolderFactory = std::function<KisAnimatedTransformParamsHolderInterfaceSP (KisDefaultBoundsBaseSP)>;

class KRITAIMAGE_EXPORT KisTransformMaskParamsFactoryRegistry
{

public:
    KisTransformMaskParamsFactoryRegistry();
    ~KisTransformMaskParamsFactoryRegistry();

    void addFactory(const QString &id, const KisTransformMaskParamsFactory &factory);
    KisTransformMaskParamsInterfaceSP createParams(const QString &id, const QDomElement &e);

    void setAnimatedParamsHolderFactory(const KisAnimatedTransformMaskParamsHolderFactory &factory);
    KisAnimatedTransformParamsHolderInterfaceSP createAnimatedParamsHolder(KisDefaultBoundsBaseSP defaultBounds);

    static KisTransformMaskParamsFactoryRegistry* instance();

private:
    KisTransformMaskParamsFactoryMap m_map;
    KisAnimatedTransformMaskParamsHolderFactory m_animatedParamsFactory;
};

#endif /* __KIS_TRANSFORM_MASK_PARAMS_FACTORY_REGISTRY_H */
