/*
 *  Copyright (c) 2013 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __KIS_OFFSET_PROCESSING_VISITOR_H
#define __KIS_OFFSET_PROCESSING_VISITOR_H

#include "kis_processing_visitor.h"
#include <QRect>


class KisOffsetProcessingVisitor : public KisProcessingVisitor
{
public:
    KisOffsetProcessingVisitor(const QPoint &offsetPoint, const QRect &wrapRect);

    void visit(KisNode *node, KisUndoAdapter *undoAdapter);
    void visit(KisPaintLayer *layer, KisUndoAdapter *undoAdapter);
    void visit(KisGroupLayer *layer, KisUndoAdapter *undoAdapter);
    void visit(KisAdjustmentLayer *layer, KisUndoAdapter *undoAdapter);
    void visit(KisExternalLayer *layer, KisUndoAdapter *undoAdapter);
    void visit(KisGeneratorLayer *layer, KisUndoAdapter *undoAdapter);
    void visit(KisCloneLayer *layer, KisUndoAdapter *undoAdapter);
    void visit(KisFilterMask *mask, KisUndoAdapter *undoAdapter);
    void visit(KisTransparencyMask *mask, KisUndoAdapter *undoAdapter);
    void visit(KisSelectionMask *mask, KisUndoAdapter *undoAdapter);

private:
    void offsetNode(KisNode *node, KisUndoAdapter *undoAdapter);

private:
    QPoint m_offset;
    QRect m_wrapRect;
};

#endif /* __KIS_OFFSET_PROCESSING_VISITOR_H */
