/*
 *  SPDX-FileCopyrightText: 2006 Boudewijn Rempt <boud@valdyas.org>
 *
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef _KIS_TRANSPARENCY_MASK_
#define _KIS_TRANSPARENCY_MASK_

#include "kis_types.h"
#include "kis_effect_mask.h"

class QRect;

/**
 *  A transparency mask is a single channel mask that applies a particular
 *  transparency to the layer the mask belongs to. It differs from an
 *  adjustment layer in that it only works on its parent layer, while
 *  adjustment layers work on all layers below it in its layer group.
 *
 *  XXX: Use KisConfig::useProjections() to enable/disable the caching of
 *       the projection.
 */
class KRITAIMAGE_EXPORT KisTransparencyMask : public KisEffectMask
{
    Q_OBJECT

public:

    KisTransparencyMask(KisImageWSP image, const QString &name);
    KisTransparencyMask(const KisTransparencyMask& rhs);
    ~KisTransparencyMask() override;

    KisNodeSP clone() const override {
        return KisNodeSP(new KisTransparencyMask(*this));
    }

    QRect decorateRect(KisPaintDeviceSP &src, KisPaintDeviceSP &dst,
                       const QRect & rc,
                       PositionToFilthy maskPos) const override;
    QIcon icon() const override;
    bool accept(KisNodeVisitor &v) override;
    void accept(KisProcessingVisitor &visitor, KisUndoAdapter *undoAdapter) override;

    QRect extent() const override;
    QRect exactBounds() const override;

    QRect changeRect(const QRect &rect, PositionToFilthy pos = N_FILTHY) const override;
    QRect needRect(const QRect &rect, PositionToFilthy pos = N_FILTHY) const override;

    bool paintsOutsideSelection() const override;
};

#endif //_KIS_TRANSPARENCY_MASK_
