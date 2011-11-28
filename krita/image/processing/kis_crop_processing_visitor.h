/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_CROP_PROCESSING_VISITOR_H
#define __KIS_CROP_PROCESSING_VISITOR_H

#include "kis_processing_visitor.h"
#include <QRect>


class KRITAIMAGE_EXPORT  KisCropProcessingVisitor : public KisProcessingVisitor
{
public:
    KisCropProcessingVisitor(const QRect &rect, bool cropLayers, bool moveLayers);

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
    void cropNode(KisNode *node, KisUndoAdapter *undoAdapter);

private:
    QRect m_rect;
    bool m_cropLayers;
    bool m_moveLayers;
};

#endif /* __KIS_CROP_PROCESSING_VISITOR_H */
