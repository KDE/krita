/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_TRANSFORM_MASK_PARAMS_INTERFACE_H
#define __KIS_TRANSFORM_MASK_PARAMS_INTERFACE_H

#include "kritaimage_export.h"
#include "kis_types.h"
#include "kis_default_bounds_base.h"

#include <QScopedPointer>


class QTransform;
class QDomElement;
class KisKeyframeChannel;

class KisTransformMaskParamsInterface;
typedef QSharedPointer<KisTransformMaskParamsInterface> KisTransformMaskParamsInterfaceSP;
typedef QWeakPointer<KisTransformMaskParamsInterface> KisTransformMaskParamsInterfaceWSP;

class KisAnimatedTransformParamsHolderInterface;
typedef QSharedPointer<KisAnimatedTransformParamsHolderInterface> KisAnimatedTransformParamsHolderInterfaceSP;
typedef QWeakPointer<KisAnimatedTransformParamsHolderInterface> KisAnimatedTransformParamsHolderInterfaceWSP;


class KRITAIMAGE_EXPORT KisTransformMaskParamsInterface
{
public:
    virtual ~KisTransformMaskParamsInterface();

    virtual QTransform finalAffineTransform() const = 0;
    virtual bool isAffine() const = 0;

    /**
     * Hides the transform mask from the rendering stack. It is used by the
     * legacy transform tool strategy to hide the mask during the overlay
     * preview.
     */
    virtual bool isHidden() const = 0;
    virtual void setHidden(bool value) = 0;

    virtual void transformDevice(KisNodeSP node, KisPaintDeviceSP src, KisPaintDeviceSP dst, bool forceSubPixelTranslation) const = 0;

    virtual QString id() const = 0;
    virtual void toXML(QDomElement *e) const = 0;

    virtual void translateSrcAndDst(const QPointF &offset) = 0;
    virtual void transformSrcAndDst(const QTransform &t) = 0;
    virtual void translateDstSpace(const QPointF &offset) = 0;

    virtual QRect nonAffineChangeRect(const QRect &rc) = 0;
    virtual QRect nonAffineNeedRect(const QRect &rc, const QRect &srcBounds) = 0;

    virtual bool compareTransform(KisTransformMaskParamsInterfaceSP rhs) const = 0;

    virtual KisTransformMaskParamsInterfaceSP clone() const = 0;
};

class KRITAIMAGE_EXPORT KisAnimatedTransformParamsHolderInterface
{
public:
    virtual ~KisAnimatedTransformParamsHolderInterface();

    virtual bool isAnimated() const = 0;

    virtual KisKeyframeChannel* requestKeyframeChannel(const QString &id) = 0;
    virtual KisKeyframeChannel* getKeyframeChannel(const QString &id) const = 0;

    virtual KisTransformMaskParamsInterfaceSP bakeIntoParams() const = 0;
    virtual void setParamsAtCurrentPosition(const KisTransformMaskParamsInterface *params, KUndo2Command *parentCommand) = 0;

    virtual KisAnimatedTransformParamsHolderInterfaceSP clone() const = 0;

    virtual void setDefaultBounds(KisDefaultBoundsBaseSP bounds) = 0;
    virtual KisDefaultBoundsBaseSP defaultBounds() const = 0;

    virtual void syncLodCache() = 0;

};

#endif /* __KIS_TRANSFORM_MASK_PARAMS_INTERFACE_H */
