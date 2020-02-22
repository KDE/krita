/*
 *  Copyright (c) 2020 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KISTRIMPROCESSINGVISITOR_H
#define KISTRIMPROCESSINGVISITOR_H

#include <QRect>

#include <kis_simple_processing_visitor.h>
#include <kis_types.h>

/**
 * @brief The KisTrimProcessingVisitor class selects all opaque pixels
 * in the layers it visits and then
 */
class KisTrimProcessingVisitor : public KisSimpleProcessingVisitor
{
public:
    KisTrimProcessingVisitor(const QRect &imageBounds);

private:
    void visitNodeWithPaintDevice(KisNode *node, KisUndoAdapter *undoAdapter) override;
    void visitExternalLayer(KisExternalLayer *layer, KisUndoAdapter *undoAdapter) override;

public:

    void visit(KisTransformMask *mask, KisUndoAdapter *undoAdapter) override;
    void visitColorizeMask(KisColorizeMask *mask, KisUndoAdapter *undoAdapter) override;
    using KisSimpleProcessingVisitor::visit;

private:
    void trimPaintDeviceImpl(KisPaintDeviceSP device, KisUndoAdapter *undoAdapter);

    QRect m_imageBounds;
};

#endif // KISTRIMPROCESSINGVISITOR_H
