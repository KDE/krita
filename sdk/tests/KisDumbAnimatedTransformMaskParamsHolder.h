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

    KisKeyframeChannel* requestKeyframeChannel(const QString &id);
    KisKeyframeChannel* getKeyframeChannel(const QString &id) const;

    KisTransformMaskParamsInterfaceSP bakeIntoParams() const;

    void setParamsAtCurrentPosition(const KisTransformMaskParamsInterface *params, KUndo2Command *parentCommand);

    KisAnimatedTransformParamsHolderInterfaceSP clone() const;

    void setDefaultBounds(KisDefaultBoundsBaseSP bounds);

    KisDefaultBoundsBaseSP defaultBounds() const;

    void syncLodCache();

private:
    KisDefaultBoundsBaseSP m_defaultBounds;
    KisTransformMaskParamsInterfaceSP m_params;
};
#endif // KISDUMBANIMATEDTRANSFORMMASKPARAMSHOLDER_H
