/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISDUMBANIMATEDTRANSFORMMASKPARAMSHOLDER_H
#define KISDUMBANIMATEDTRANSFORMMASKPARAMSHOLDER_H

#include "kritatransformmaskstubs_export.h"
#include "kis_transform_mask_params_interface.h"


class KRITATRANSFORMMASKSTUBS_EXPORT KisDumbAnimatedTransformMaskParamsHolder : public KisAnimatedTransformParamsHolderInterface
{
public:
    KisDumbAnimatedTransformMaskParamsHolder(KisDefaultBoundsBaseSP bounds);

    KisDumbAnimatedTransformMaskParamsHolder(const KisDumbAnimatedTransformMaskParamsHolder &rhs);

    bool isAnimated() const override;

    KisKeyframeChannel* requestKeyframeChannel(const QString &id) override;
    KisKeyframeChannel* getKeyframeChannel(const QString &id) const override;

    KisTransformMaskParamsInterfaceSP bakeIntoParams() const override;

    void setParamsAtCurrentPosition(const KisTransformMaskParamsInterface *params, KUndo2Command *parentCommand) override;

    KisAnimatedTransformParamsHolderInterfaceSP clone() const override;

    void setDefaultBounds(KisDefaultBoundsBaseSP bounds) override;

    KisDefaultBoundsBaseSP defaultBounds() const override;

    void syncLodCache() override;

private:
    KisDefaultBoundsBaseSP m_defaultBounds;
    KisTransformMaskParamsInterfaceSP m_params;
};
#endif // KISDUMBANIMATEDTRANSFORMMASKPARAMSHOLDER_H
