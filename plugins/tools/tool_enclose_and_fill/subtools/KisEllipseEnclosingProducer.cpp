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

#include "KisEllipseEnclosingProducer.h"

KisEllipseEnclosingProducer::KisEllipseEnclosingProducer(KoCanvasBase * canvas)
    : KisDynamicDelegateTool<KisToolEllipseBase>(canvas, KisToolEllipseBase::PAINT, KisCursor::load("tool_elliptical_selection_cursor.png", 6, 6))
{
    setObjectName("enclosing_tool_rectangle");
    setSupportOutline(true);
    setOutlineEnabled(false);

    KisCanvas2 *kritaCanvas = dynamic_cast<KisCanvas2*>(canvas);

    connect(kritaCanvas->viewManager()->canvasResourceProvider(), SIGNAL(sigEffectiveCompositeOpChanged()), SLOT(resetCursorStyle()));
}

KisEllipseEnclosingProducer::~KisEllipseEnclosingProducer()
{}

void  KisEllipseEnclosingProducer::resetCursorStyle()
{
    if (isEraser()) {
        useCursor(KisCursor::load("tool_elliptical_selection_enclose_eraser_cursor.png", 6, 6));
    } else {
        KisDynamicDelegateTool::resetCursorStyle();
    }

    overrideCursorIfNotEditable();
}

void KisEllipseEnclosingProducer::finishRect(const QRectF& rect, qreal roundCornersX, qreal roundCornersY)
{
    Q_UNUSED(roundCornersX);
    Q_UNUSED(roundCornersY);
    
    QRect rc(rect.normalized().toRect());
    if (!rc.isValid()) {
        return;
    }

    KisPixelSelectionSP enclosingMask = KisPixelSelectionSP(new KisPixelSelection());
    QPainterPath path;

    path.addEllipse(rc);
    getRotatedPath(path, rc.center(), getRotationAngle());

    KisPainter painter(enclosingMask);
    painter.setPaintColor(KoColor(Qt::white, enclosingMask->colorSpace()));
    painter.setAntiAliasPolygonFill(false);
    painter.setFillStyle(KisPainter::FillStyleForegroundColor);
    painter.setStrokeStyle(KisPainter::StrokeStyleNone);

    painter.paintPainterPath(path);

    Q_EMIT enclosingMaskProduced(enclosingMask);
}

bool KisEllipseEnclosingProducer::hasUserInteractionRunning() const
{
    return m_hasUserInteractionRunning;
}

void KisEllipseEnclosingProducer::beginShape()
{
    m_hasUserInteractionRunning = true;
}

void KisEllipseEnclosingProducer::endShape()
{
    m_hasUserInteractionRunning = false;
}
