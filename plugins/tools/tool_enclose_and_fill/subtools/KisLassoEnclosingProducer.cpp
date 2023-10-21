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

#include "KisLassoEnclosingProducer.h"

KisLassoEnclosingProducer::KisLassoEnclosingProducer(KoCanvasBase * canvas)
    : KisDynamicDelegateTool<KisToolOutlineBase>(canvas, KisToolOutlineBase::PAINT, KisCursor::load("tool_outline_selection_cursor.png", 5, 5))
{
    setObjectName("enclosing_tool_lasso");
    setSupportOutline(true);
    setOutlineEnabled(false);

    KisCanvas2 *kritaCanvas = dynamic_cast<KisCanvas2*>(canvas);

    connect(kritaCanvas->viewManager()->canvasResourceProvider(), SIGNAL(sigEffectiveCompositeOpChanged()), SLOT(resetCursorStyle()));
}

KisLassoEnclosingProducer::~KisLassoEnclosingProducer()
{}

void  KisLassoEnclosingProducer::resetCursorStyle()
{
    if (isEraser()) {
        useCursor(KisCursor::load("tool_outline_selection_enclose_eraser_cursor.png", 5, 5));
    } else {
        KisDynamicDelegateTool::resetCursorStyle();
    }

    overrideCursorIfNotEditable();
}

void KisLassoEnclosingProducer::finishOutline(const QVector<QPointF> &points)
{
    if (points.size() < 3) {
        return;
    }
    
    KisPixelSelectionSP enclosingMask = new KisPixelSelection();

    KisPainter painter(enclosingMask);
    painter.setPaintColor(KoColor(Qt::white, enclosingMask->colorSpace()));
    painter.setAntiAliasPolygonFill(false);
    painter.setFillStyle(KisPainter::FillStyleForegroundColor);
    painter.setStrokeStyle(KisPainter::StrokeStyleNone);

    painter.paintPolygon(points);

    emit enclosingMaskProduced(enclosingMask);
}

bool KisLassoEnclosingProducer::hasUserInteractionRunning() const
{
    return m_hasUserInteractionRunning;
}

void KisLassoEnclosingProducer::beginShape()
{
    m_hasUserInteractionRunning = true;
}

void KisLassoEnclosingProducer::endShape()
{
    m_hasUserInteractionRunning = false;
}
