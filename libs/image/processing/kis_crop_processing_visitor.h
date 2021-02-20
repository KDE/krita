/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_CROP_PROCESSING_VISITOR_H
#define __KIS_CROP_PROCESSING_VISITOR_H

#include "kis_simple_processing_visitor.h"
#include <QRect>
#include "kis_types.h"


class KRITAIMAGE_EXPORT  KisCropProcessingVisitor : public KisSimpleProcessingVisitor
{
public:
    KisCropProcessingVisitor(const QRect &rect, bool cropLayers, bool moveLayers);

private:
    void visitNodeWithPaintDevice(KisNode *node, KisUndoAdapter *undoAdapter) override;
    void visitExternalLayer(KisExternalLayer *layer, KisUndoAdapter *undoAdapter) override;

public:

    void visit(KisTransformMask *mask, KisUndoAdapter *undoAdapter) override;
    void visitColorizeMask(KisColorizeMask *mask, KisUndoAdapter *undoAdapter) override;
    using KisSimpleProcessingVisitor::visit;

private:
    void moveNodeImpl(KisNode *node, KisUndoAdapter *undoAdapter);
    void cropPaintDeviceImpl(KisPaintDeviceSP device, KisUndoAdapter *undoAdapter);

private:
    QRect m_rect;
    bool m_cropLayers;
    bool m_moveLayers;
};

#endif /* __KIS_CROP_PROCESSING_VISITOR_H */
