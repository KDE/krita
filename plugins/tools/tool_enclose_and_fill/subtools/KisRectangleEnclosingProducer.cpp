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

#include "KisRectangleEnclosingProducer.h"

KisRectangleEnclosingProducer::KisRectangleEnclosingProducer(KoCanvasBase * canvas)
    : KisDynamicDelegateTool<KisToolRectangleBase>(canvas, KisToolRectangleBase::PAINT, KisCursor::load("tool_rectangular_selection_cursor.png", 6, 6))
{
    setObjectName("enclosing_tool_rectangle");
    setSupportOutline(true);
    setOutlineEnabled(false);

    KisCanvas2 *kritaCanvas = dynamic_cast<KisCanvas2*>(canvas);

    connect(kritaCanvas->viewManager()->canvasResourceProvider(), SIGNAL(sigEffectiveCompositeOpChanged()), SLOT(resetCursorStyle()));
}

KisRectangleEnclosingProducer::~KisRectangleEnclosingProducer()
{}

void  KisRectangleEnclosingProducer::resetCursorStyle()
{
    if (isEraser()) {
        useCursor(KisCursor::load("tool_rectangular_selection_enclose_eraser_cursor.png", 6, 6));
    } else {
        KisDynamicDelegateTool::resetCursorStyle();
    }

    overrideCursorIfNotEditable();
}

void KisRectangleEnclosingProducer::finishRect(const QRectF& rect, qreal roundCornersX, qreal roundCornersY)
{
    QRect rc(rect.normalized().toRect());
    if (!rc.isValid()) {
        return;
    }

    KisPixelSelectionSP enclosingMask = KisPixelSelectionSP(new KisPixelSelection());
    QPainterPath path;

    if (roundCornersX > 0 || roundCornersY > 0) {
        path.addRoundedRect(rc, roundCornersX, roundCornersY);
    } else {
        path.addRect(rc);
    }
    getRotatedPath(path, rc.center(), getRotationAngle());

    KisPainter painter(enclosingMask);
    painter.setPaintColor(KoColor(Qt::white, enclosingMask->colorSpace()));
    painter.setAntiAliasPolygonFill(false);
    painter.setFillStyle(KisPainter::FillStyleForegroundColor);
    painter.setStrokeStyle(KisPainter::StrokeStyleNone);

    painter.paintPainterPath(path);

    emit enclosingMaskProduced(enclosingMask);
}

bool KisRectangleEnclosingProducer::hasUserInteractionRunning() const
{
    return m_hasUserInteractionRunning;
}

void KisRectangleEnclosingProducer::beginShape()
{
    m_hasUserInteractionRunning = true;
}

void KisRectangleEnclosingProducer::endShape()
{
    m_hasUserInteractionRunning = false;
}
