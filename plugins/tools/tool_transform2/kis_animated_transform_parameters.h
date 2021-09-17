/*
 *  SPDX-FileCopyrightText: 2016 Jouni Pentikäinen <joupent@gmail.com>
 *  SPDX-FileCopyrightText: 2021 Eoin O'Neill<eoinoneill1991@gmail.com>
 *  SPDX-FileCopyrightText: 2021 Emmet O'Neill <emmetoneill.pdx@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */


#ifndef __KIS_ANIMATED_TRANSFORM_MASK_PARAMETERS_H
#define __KIS_ANIMATED_TRANSFORM_MASK_PARAMETERS_H

#include "kis_transform_mask_adapter.h"
#include "kritatooltransform_export.h"
#include <qmath.h>

class KisKeyframeChannel;

class KRITATOOLTRANSFORM_EXPORT KisAnimatedTransformMaskParameters : public KisTransformMaskAdapter, public KisAnimatedTransformParamsInterface
{
public:
    KisAnimatedTransformMaskParameters();
    KisAnimatedTransformMaskParameters(const KisTransformMaskAdapter *staticTransform);
    KisAnimatedTransformMaskParameters(const KisAnimatedTransformMaskParameters& rhs);
    ~KisAnimatedTransformMaskParameters() override;

    const QSharedPointer<ToolTransformArgs> transformArgs() const override;

    QString id() const override;
    void toXML(QDomElement *e) const override;

    KisKeyframeChannel *requestKeyframeChannel(const QString &id, KisNodeWSP parent) override;
    void setKeyframeChannel(const QString &name, QSharedPointer<KisKeyframeChannel> kcsp) override;
    KisKeyframeChannel* getKeyframeChannel(const KoID& koid) const override;
    QList<KisKeyframeChannel*> copyChannelsFrom(const KisAnimatedTransformParamsInterface *other) override;

    void initializeKeyframes(KisTransformMaskSP mask, KisTransformMaskParamsInterfaceSP params, KUndo2Command* cmnd = nullptr) override;
    void setKeyframeData(KisTransformMaskSP mask, KisTransformMaskParamsInterfaceSP params, KUndo2Command* cmnd = nullptr) override;

    qreal defaultValueForScalarChannel(QString name);

    bool isHidden() const override;
    void setHidden(bool hidden);

    void clearChangedFlag() override;
    bool hasChanged() const override;
    bool isAnimated() const;

    QPointF getRotationalTranslationOffset(const ToolTransformArgs& args) const;

    KisTransformMaskParamsInterfaceSP clone() const override;

    static KisTransformMaskParamsInterfaceSP fromXML(const QDomElement &e);

    /*** Some utility methods for creating an animated transform mask and for creating keyframes using a reference
     * set of parameters. Used by the transform mask and stroke respectively to update keyframe data. */
    static KisTransformMaskParamsInterfaceSP makeAnimated(KisTransformMaskParamsInterfaceSP params, const KisTransformMaskSP mask);
    static void addKeyframes(KisTransformMaskSP mask, int currentTime, KisTransformMaskParamsInterfaceSP desiredParams, KUndo2Command *parentCommand);
    static void setKeyframes(KisTransformMaskSP mask, int currentTime, KisTransformMaskParamsInterfaceSP desiredParams, KUndo2Command *parentCommand);
    static void removeKeyframes(KisTransformMaskSP mask, int currentTime);
    static qreal degToRad(qreal degrees) {return degrees / 180 * M_PI;}
    static qreal radToDeg(qreal rad) {return rad * 180 / M_PI;}

private:
    quint64 generateStateHash() const;

    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif
