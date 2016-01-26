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

#include "processing/kis_simple_processing_visitor.h"
#include <QRect>


class KisOffsetProcessingVisitor : public KisSimpleProcessingVisitor
{
public:
    KisOffsetProcessingVisitor(const QPoint &offsetPoint, const QRect &wrapRect);

    void visitNodeWithPaintDevice(KisNode *node, KisUndoAdapter *undoAdapter);
    void visitExternalLayer(KisExternalLayer *layer, KisUndoAdapter *undoAdapter);

private:
    QPoint m_offset;
    QRect m_wrapRect;
};

#endif /* __KIS_OFFSET_PROCESSING_VISITOR_H */
