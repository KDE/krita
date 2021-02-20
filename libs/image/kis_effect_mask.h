/*
 *  SPDX-FileCopyrightText: 2006 Boudewijn Rempt <boud@valdyas.org>
 *
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef _KIS_EFFECT_MASK_
#define _KIS_EFFECT_MASK_

#include "kis_mask.h"
/**
 * An effect mask is a single channel mask that applies a particular
 * effect to the layer the mask belongs to. It differs from an
 * adjustment layer in that it only works on its parent layer, while
 * adjustment layers work on all layers below it in its layer group.
 * The effect could be a filter, a transformation or anything else
 * that messes up pixels.
 */
class KRITAIMAGE_EXPORT KisEffectMask : public KisMask
{

    Q_OBJECT

public:

    /**
     * Create an empty effect mask.
     */
    KisEffectMask(KisImageWSP image, const QString &name);

    ~KisEffectMask() override;
    KisEffectMask(const KisEffectMask& rhs);

    QIcon icon() const override;

    using KisMask::apply;
};

#endif //_KIS_EFFECT_MASK_
