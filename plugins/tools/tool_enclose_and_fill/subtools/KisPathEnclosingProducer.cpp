/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2022 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <kis_cursor.h>
#include <KoPathShape.h>
#include <KisViewManager.h>
#include <KoIcon.h>

#include "KisPathEnclosingProducer.h"

KisToolPathLocalTool::KisToolPathLocalTool(KoCanvasBase * canvas, KisPathEnclosingProducer* parentTool)
    : KoCreatePathTool(canvas)
    , m_parentTool(parentTool)
{}

void KisToolPathLocalTool::paintPath(KoPathShape &pathShape, QPainter &painter, const KoViewConverter &converter)
{
    Q_UNUSED(converter);

    QTransform matrix;
    matrix.scale(m_parentTool->image()->xRes(), m_parentTool->image()->yRes());
    matrix.translate(pathShape.position().x(), pathShape.position().y());
    m_parentTool->paintToolOutline(&painter, m_parentTool->pixelToView(matrix.map(pathShape.outline())));
}

void KisToolPathLocalTool::addPathShape(KoPathShape* pathShape)
{
    m_parentTool->addPathShape(pathShape);
}

void KisToolPathLocalTool::beginShape()
{
    m_parentTool->beginShape();
}

void KisToolPathLocalTool::endShape()
{
    m_parentTool->endShape();
}

KisPathEnclosingProducer::KisPathEnclosingProducer(KoCanvasBase * canvas)
    : KisDynamicDelegateTool<DelegatedPathTool>(canvas,
                                                KisCursor::load("tool_polygonal_selection_cursor.png", 6, 6),
                                                new KisToolPathLocalTool(canvas, this))
{
    setObjectName("enclosing_tool_path");
    setSupportOutline(true);
    setOutlineEnabled(false);
}

KisPathEnclosingProducer::~KisPathEnclosingProducer()
{}

void KisPathEnclosingProducer::requestStrokeEnd()
{
    KisDynamicDelegateTool::requestStrokeEnd();
    localTool()->endPathWithoutLastPoint();
}

void KisPathEnclosingProducer::requestStrokeCancellation()
{
    KisDynamicDelegateTool::requestStrokeCancellation();
    localTool()->cancelPath();
}

void KisPathEnclosingProducer::mousePressEvent(KoPointerEvent *event)
{
    Q_UNUSED(event)
}

// Install an event filter to catch right-click events.
// The simplest way to accommodate the popup palette binding.
// This code is duplicated in kis_tool_select_path.cc
bool KisPathEnclosingProducer::eventFilter(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj);
    if (event->type() == QEvent::MouseButtonPress ||
            event->type() == QEvent::MouseButtonDblClick) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::RightButton) {
            localTool()->removeLastPoint();
            return true;
        }
    } else if (event->type() == QEvent::TabletPress) {
        QTabletEvent *tabletEvent = static_cast<QTabletEvent*>(event);
        if (tabletEvent->button() == Qt::RightButton) {
            localTool()->removeLastPoint();
            return true;
        }
    }
    return false;
}

void KisPathEnclosingProducer::beginAlternateAction(KoPointerEvent *event, AlternateAction action) {
    KisDynamicDelegateTool::beginAlternateAction(event, action);
    if (!nodeEditable()) return;

    if (nodePaintAbility() == KisDynamicDelegateTool::MYPAINTBRUSH_UNPAINTABLE) {
        KisCanvas2 * kiscanvas = static_cast<KisCanvas2*>(canvas());
        QString message = i18n("The MyPaint Brush Engine is not available for this colorspace");
        kiscanvas->viewManager()->showFloatingMessage(message, koIcon("object-locked"));
        event->ignore();
        return;
    }
}

void KisPathEnclosingProducer::beginPrimaryAction(KoPointerEvent* event)
{
    if (!nodeEditable()) return;
    KisDynamicDelegateTool::mousePressEvent(event);
}

void KisPathEnclosingProducer::continuePrimaryAction(KoPointerEvent *event)
{
    mouseMoveEvent(event);
}

void KisPathEnclosingProducer::endPrimaryAction(KoPointerEvent *event)
{
    mouseReleaseEvent(event);
}

void KisPathEnclosingProducer::beginPrimaryDoubleClickAction(KoPointerEvent *event)
{
    KisDynamicDelegateTool::mouseDoubleClickEvent(event);
}

void KisPathEnclosingProducer::addPathShape(KoPathShape* pathShape)
{
    KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    if (!kisCanvas) {
        return;
    }

    KisImageWSP image = kisCanvas->image();
    KisPixelSelectionSP enclosingMask = new KisPixelSelection();

    pathShape->normalize();
    pathShape->close();

    KisPainter painter(enclosingMask);
    painter.setPaintColor(KoColor(Qt::white, enclosingMask->colorSpace()));
    painter.setAntiAliasPolygonFill(false);
    painter.setFillStyle(KisPainter::FillStyleForegroundColor);
    painter.setStrokeStyle(KisPainter::StrokeStyleNone);

    QTransform matrix;
    matrix.scale(image->xRes(), image->yRes());
    matrix.translate(pathShape->position().x(), pathShape->position().y());

    QPainterPath path = matrix.map(pathShape->outline());
    painter.fillPainterPath(path);
    enclosingMask->setOutlineCache(path);

    delete pathShape;

    emit enclosingMaskProduced(enclosingMask);
}

bool KisPathEnclosingProducer::hasUserInteractionRunning() const
{
    return m_hasUserInteractionRunning;
}

void KisPathEnclosingProducer::beginShape()
{
    m_hasUserInteractionRunning = true;
}

void KisPathEnclosingProducer::endShape()
{
    m_hasUserInteractionRunning = false;
}
