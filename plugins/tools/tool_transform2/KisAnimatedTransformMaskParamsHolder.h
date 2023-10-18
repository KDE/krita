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

class KRITATOOLTRANSFORM_EXPORT KisAnimatedTransformMaskParamsHolder : public KisAnimatedTransformParamsHolderInterface
{
public:
    KisAnimatedTransformMaskParamsHolder(KisDefaultBoundsBaseSP defaultBounds);
    KisAnimatedTransformMaskParamsHolder(const KisAnimatedTransformMaskParamsHolder& rhs);
    ~KisAnimatedTransformMaskParamsHolder() override;

    bool isAnimated() const override;

    const QSharedPointer<ToolTransformArgs> transformArgs() const;

    void setDefaultBounds(KisDefaultBoundsBaseSP bounds) override;
    KisDefaultBoundsBaseSP defaultBounds() const override;

    KisKeyframeChannel *requestKeyframeChannel(const QString &id) override;
    KisKeyframeChannel* getKeyframeChannel(const QString &id) const override;

    void syncLodCache() override;

    KisAnimatedTransformParamsHolderInterfaceSP clone() const override;

    KisTransformMaskParamsInterfaceSP bakeIntoParams() const override;
    void setParamsAtCurrentPosition(const KisTransformMaskParamsInterface *params, KUndo2Command *parentCommand) override;

private:
    void setNewTransformArgs(const ToolTransformArgs &args, KUndo2Command *parentCommand);
    qreal defaultValueForScalarChannel(const KoID &id);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif
