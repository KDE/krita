/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2022 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisBrushEnclosingProducer.h"

KisBrushEnclosingProducer::KisBrushEnclosingProducer(KoCanvasBase * canvas)
    : KisDynamicDelegateTool<KisToolBasicBrushBase>(canvas, KisToolBasicBrushBase::PAINT, KisCursor::load("tool_freehand_cursor.xpm", 2, 2))
{
    setObjectName("enclosing_tool_brush");
}

KisBrushEnclosingProducer::~KisBrushEnclosingProducer()
{}

void KisBrushEnclosingProducer::finishStroke(const QPainterPath &stroke)
{
    if (stroke.isEmpty()) {
        return;
    }
    
    KisPixelSelectionSP enclosingMask = new KisPixelSelection();

    KisPainter painter(enclosingMask);
    painter.setPaintColor(KoColor(Qt::white, enclosingMask->colorSpace()));
    painter.setAntiAliasPolygonFill(false);
    painter.setFillStyle(KisPainter::FillStyleForegroundColor);
    painter.setStrokeStyle(KisPainter::StrokeStyleNone);

    painter.fillPainterPath(stroke);

    emit enclosingMaskProduced(enclosingMask);
}

bool KisBrushEnclosingProducer::hasUserInteractionRunning() const
{
    return m_hasUserInteractionRunning;
}

void KisBrushEnclosingProducer::beginShape()
{
    m_hasUserInteractionRunning = true;
}

void KisBrushEnclosingProducer::endShape()
{
    m_hasUserInteractionRunning = false;
}
