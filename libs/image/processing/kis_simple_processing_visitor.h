/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_SIMPLE_PROCESSING_VISITOR_H
#define __KIS_SIMPLE_PROCESSING_VISITOR_H

#include "kis_processing_visitor.h"


class KRITAIMAGE_EXPORT KisSimpleProcessingVisitor : public KisProcessingVisitor
{
public:
    ~KisSimpleProcessingVisitor() override;

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

protected:
    virtual void visitNodeWithPaintDevice(KisNode *node, KisUndoAdapter *undoAdapter) = 0;
    virtual void visitExternalLayer(KisExternalLayer *layer, KisUndoAdapter *undoAdapter) = 0;
    virtual void visitColorizeMask(KisColorizeMask *mask, KisUndoAdapter *undoAdapter) = 0;
};

#endif /* __KIS_SIMPLE_PROCESSING_VISITOR_H */
