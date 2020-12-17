/*
 *  SPDX-FileCopyrightText: 2013 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_OFFSET_PROCESSING_VISITOR_H
#define __KIS_OFFSET_PROCESSING_VISITOR_H

#include "processing/kis_simple_processing_visitor.h"
#include <QRect>
#include "kis_types.h"


class KisOffsetProcessingVisitor : public KisSimpleProcessingVisitor
{
public:
    KisOffsetProcessingVisitor(const QPoint &offsetPoint, const QRect &wrapRect);

    void visitNodeWithPaintDevice(KisNode *node, KisUndoAdapter *undoAdapter) override;
    void visitExternalLayer(KisExternalLayer *layer, KisUndoAdapter *undoAdapter) override;
    void visitColorizeMask(KisColorizeMask *mask, KisUndoAdapter *undoAdapter) override;

private:
    void offsetPaintDevice(KisPaintDeviceSP device, KisUndoAdapter *undoAdapter);
private:
    QPoint m_offset;
    QRect m_wrapRect;
};

#endif /* __KIS_OFFSET_PROCESSING_VISITOR_H */
