/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef _KIS_FILTER_MASK_
#define _KIS_FILTER_MASK_

#include "kis_types.h"
#include "kis_effect_mask.h"

#include "kis_node_filter_interface.h"
#include "kis_filter_configuration.h"


/**
   An filter mask is a single channel mask that applies a particular
   filter to the layer the mask belongs to. It differs from an
   adjustment layer in that it only works on its parent layer, while
   adjustment layers work on all layers below it in its layer group.
*/

class KRITAIMAGE_EXPORT KisFilterMask : public KisEffectMask, public KisNodeFilterInterface
{
    Q_OBJECT

public:

    /**
     * Create an empty filter mask.
     */
    KisFilterMask(KisImageWSP image, const QString &name = QString());

    ~KisFilterMask() override;

    QIcon icon() const override;

    KisNodeSP clone() const override {
        return KisNodeSP(new KisFilterMask(*this));
    }

    bool accept(KisNodeVisitor &v) override;
    void accept(KisProcessingVisitor &visitor, KisUndoAdapter *undoAdapter) override;

    KisFilterMask(const KisFilterMask& rhs);

    void setFilter(KisFilterConfigurationSP filterConfig) override;

    QRect decorateRect(KisPaintDeviceSP &src,
                       KisPaintDeviceSP &dst,
                       const QRect & rc,
                       PositionToFilthy maskPos) const override;

    QRect changeRect(const QRect &rect, PositionToFilthy pos = N_FILTHY) const override;
    QRect needRect(const QRect &rect, PositionToFilthy pos = N_FILTHY) const override;
};

#endif //_KIS_FILTER_MASK_
