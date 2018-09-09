/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_MIRROR_PROCESSING_VISITOR_H
#define __KIS_MIRROR_PROCESSING_VISITOR_H

#include "kis_simple_processing_visitor.h"
#include <QRect>
#include "kis_types.h"

#include "KisSelectionBasedProcessingHelper.h"


class KRITAIMAGE_EXPORT KisMirrorProcessingVisitor : public KisSimpleProcessingVisitor
{
public:
    KisMirrorProcessingVisitor(const QRect &bounds, Qt::Orientation orientation);
    KisMirrorProcessingVisitor(KisSelectionSP selection, Qt::Orientation orientation);

private:
    void visitNodeWithPaintDevice(KisNode *node, KisUndoAdapter *undoAdapter) override;
    void visitExternalLayer(KisExternalLayer *layer, KisUndoAdapter *undoAdapter) override;

    void visitColorizeMask(KisColorizeMask *node, KisUndoAdapter *undoAdapter) override;

    KUndo2Command* createInitCommand() override;

    void mirrorDevice(KisPaintDeviceSP device);

private:
    void transformPaintDevice(KisPaintDeviceSP device, KisUndoAdapter *undoAdapter);

    QRect m_bounds;
    Qt::Orientation m_orientation;
    qreal m_axis = 0.0;

    KisSelectionBasedProcessingHelper m_selectionHelper;
};

#endif /* __KIS_MIRROR_PROCESSING_VISITOR_H */
