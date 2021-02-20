/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_DO_NOTHING_PROCESSING_VISITOR_H
#define __KIS_DO_NOTHING_PROCESSING_VISITOR_H

#include "kis_processing_visitor.h"


class KRITAIMAGE_EXPORT KisDoNothingProcessingVisitor : public KisProcessingVisitor
{
public:
    ~KisDoNothingProcessingVisitor() override;

    void visit(KisNode *node, KisUndoAdapter *undoAdapter) override;
    void visit(KisPaintLayer *layer, KisUndoAdapter *undoAdapter) override;
    void visit(KisGroupLayer *layer, KisUndoAdapter *undoAdapter) override;
    void visit(KisAdjustmentLayer *layer, KisUndoAdapter *undoAdapter) override;
    void visit(KisExternalLayer *layer, KisUndoAdapter *undoAdapter) override;
    void visit(KisGeneratorLayer *layer, KisUndoAdapter *undoAdapter) override;
    void visit(KisCloneLayer *layer, KisUndoAdapter *undoAdapter) override;
    void visit(KisFilterMask *mask, KisUndoAdapter *undoAdapter) override;
    void visit(KisTransformMask *mask, KisUndoAdapter *undoAdapter) override;
    void visit(KisTransparencyMask *mask, KisUndoAdapter *undoAdapter) override;
    void visit(KisSelectionMask *mask, KisUndoAdapter *undoAdapter) override;
    void visit(KisColorizeMask *mask, KisUndoAdapter *undoAdapter) override;
};

#endif /* __KIS_DO_NOTHING_PROCESSING_VISITOR_H */
