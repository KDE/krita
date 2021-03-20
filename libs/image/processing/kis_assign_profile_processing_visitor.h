/*
 *  SPDX-FileCopyrightText: 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_ASSIGN_PROFILE_PROCESSING_VISITOR_H
#define __KIS_ASSIGN_PROFILE_PROCESSING_VISITOR_H

#include "kis_simple_processing_visitor.h"
#include <QRect>
#include "kis_types.h"

class KoColorSpace;

class KRITAIMAGE_EXPORT  KisAssignProfileProcessingVisitor : public KisSimpleProcessingVisitor
{
public:
    KisAssignProfileProcessingVisitor(const KoColorSpace *srcColorSpace,
                                      const KoColorSpace *dstColorSpace);

private:
    void visitNodeWithPaintDevice(KisNode *node, KisUndoAdapter *undoAdapter) override;
    void visitExternalLayer(KisExternalLayer *layer, KisUndoAdapter *undoAdapter) override;

public:

    void visit(KisTransformMask *mask, KisUndoAdapter *undoAdapter) override;
    void visitColorizeMask(KisColorizeMask *mask, KisUndoAdapter *undoAdapter) override;
    using KisSimpleProcessingVisitor::visit;

private:
    const KoColorSpace *m_srcColorSpace;
    const KoColorSpace *m_dstColorSpace;
};

#endif /* __KIS_ASSIGN_PROFILE_PROCESSING_VISITOR_H */
