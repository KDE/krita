/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
