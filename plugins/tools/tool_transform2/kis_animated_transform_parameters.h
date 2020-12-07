/*
 *  Copyright (c) 2016 Jouni Pentikäinen <joupent@gmail.com>
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

    const ToolTransformArgs& transformArgs() const override;

    QString id() const override;
    void toXML(QDomElement *e) const override;
    static KisTransformMaskParamsInterfaceSP fromXML(const QDomElement &e);
    static KisTransformMaskParamsInterfaceSP animate(KisTransformMaskParamsInterfaceSP params);

    void translate(const QPointF &offset) override;

    KisKeyframeChannel *getKeyframeChannel(const QString &id, KisNodeSP defaultBounds) override;

    bool isHidden() const override;
    void setHidden(bool hidden);

    void clearChangedFlag() override;
    bool hasChanged() const override;
    bool isAnimated() const;

    static void addKeyframes(KisTransformMaskSP mask, int time, KisTransformMaskParamsInterfaceSP params, KUndo2Command *parentCommand);

    
private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif
