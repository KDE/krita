/*
 *  SPDX-FileCopyrightText: 2016 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */


#ifndef __KIS_ANIMATED_TRANSFORM_MASK_PARAMETERS_H
#define __KIS_ANIMATED_TRANSFORM_MASK_PARAMETERS_H

#include "kis_transform_mask_adapter.h"
#include "kritatooltransform_export.h"

class KisKeyframeChannel;

class KRITATOOLTRANSFORM_EXPORT KisAnimatedTransformMaskParameters : public KisTransformMaskAdapter, public KisAnimatedTransformParamsInterface
{
public:
    KisAnimatedTransformMaskParameters();
    KisAnimatedTransformMaskParameters(const KisTransformMaskAdapter *staticTransform);
    ~KisAnimatedTransformMaskParameters() override;

    const QSharedPointer<ToolTransformArgs> transformArgs() const override;

    QString id() const override;
    void toXML(QDomElement *e) const override;

    void translate(const QPointF &offset) override;

    KisKeyframeChannel *getKeyframeChannel(const QString &id, KisNodeSP defaultBounds) override;

    bool isHidden() const override;
    void setHidden(bool hidden);

    void clearChangedFlag() override;
    bool hasChanged() const override;
    bool isAnimated() const;

    static KisTransformMaskParamsInterfaceSP fromXML(const QDomElement &e);

    /*** Some utility methods for creating an animated transform mask and for creating keyframes using a reference
     * set of parameters. Used by the transform mask and stroke respectively to update keyframe data. */
    static KisTransformMaskParamsInterfaceSP makeAnimated(KisTransformMaskParamsInterfaceSP params);
    static void makeScalarKeyframeOnMask(KisTransformMaskSP mask, const KoID &channelId, int time, qreal value, KUndo2Command *parentCommand);
    static void addKeyframes(KisTransformMaskSP mask, int time, KisTransformMaskParamsInterfaceSP params, KUndo2Command *parentCommand);

    
private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif
