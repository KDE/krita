/*
 *  Copyright (c) 2016 Jouni Pentikäinen <joupent@gmail.com>
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

    KisKeyframeChannel *getKeyframeChannel(const QString &id, KisDefaultBoundsBaseSP defaultBounds) override;

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
