/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2022 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisLassoEnclosingProducer.h"

KisLassoEnclosingProducer::KisLassoEnclosingProducer(KoCanvasBase * canvas)
    : KisDynamicDelegateTool<KisToolOutlineBase>(canvas, KisToolOutlineBase::PAINT, KisCursor::load("tool_outline_selection_cursor.png", 6, 6))
{
    setObjectName("enclosing_tool_lasso");
    setSupportOutline(true);
    setOutlineEnabled(false);
}

KisLassoEnclosingProducer::~KisLassoEnclosingProducer()
{}

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
