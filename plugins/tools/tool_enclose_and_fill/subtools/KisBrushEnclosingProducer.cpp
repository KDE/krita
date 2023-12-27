/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2022 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <KisViewManager.h>
#include <canvas/kis_canvas2.h>
#include <kis_canvas_resource_provider.h>

#include "KisBrushEnclosingProducer.h"

KisBrushEnclosingProducer::KisBrushEnclosingProducer(KoCanvasBase * canvas)
    : KisDynamicDelegateTool<KisToolBasicBrushBase>(canvas, KisToolBasicBrushBase::PAINT, KisCursor::load("tool_freehand_cursor.xpm", 2, 2))
{
    setObjectName("enclosing_tool_brush");

    KisCanvas2 *kritaCanvas = dynamic_cast<KisCanvas2*>(canvas);

    connect(kritaCanvas->viewManager()->canvasResourceProvider(), SIGNAL(sigEffectiveCompositeOpChanged()), SLOT(resetCursorStyle()));
}

KisBrushEnclosingProducer::~KisBrushEnclosingProducer()
{}

void  KisBrushEnclosingProducer::resetCursorStyle()
{
    if (isEraser()) {
        useCursor(KisCursor::load("cursor-eraser.xpm", 2, 2));
    } else {
        KisDynamicDelegateTool::resetCursorStyle();
    }

    overrideCursorIfNotEditable();
}

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
