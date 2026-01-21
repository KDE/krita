/*
 *  SPDX-FileCopyrightText: 2012 Sven Langkamp <sven.langkamp@gmail.com>
 *  SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_tool_pencil.h"
#include <KoPathShape.h>
#include <KoCanvasBase.h>
#include <KoPointerEvent.h>
#include <KoShapeStroke.h>
#include <KisViewManager.h>
#include <canvas/kis_canvas2.h>
#include <kis_canvas_resource_provider.h>

#include <kis_cursor.h>

KisToolPencil::KisToolPencil(KoCanvasBase * canvas)
    : DelegatedPencilTool(canvas, Qt::ArrowCursor,
                          new __KisToolPencilLocalTool(canvas, this))
{
    setIsOpacityPresetMode(true);
    KisCanvas2 *kritaCanvas = dynamic_cast<KisCanvas2*>(canvas);

    connect(kritaCanvas->viewManager()->canvasResourceProvider(), SIGNAL(sigEffectiveCompositeOpChanged()), SLOT(resetCursorStyle()));
}

void KisToolPencil::resetCursorStyle()
{
    if (isEraser() && (nodePaintAbility() == PAINT)) {
        useCursor(KisCursor::eraserCursor());
    } else {
        DelegatedPencilTool::resetCursorStyle();
    }

    overrideCursorIfNotEditable();
}

void KisToolPencil::updatePencilCursor(bool value)
{
    if (mode() == HOVER_MODE || mode() == PAINT_MODE) {
        setCursor(value ? Qt::ArrowCursor : Qt::ForbiddenCursor);
        resetCursorStyle();
    }
}

void KisToolPencil::mousePressEvent(KoPointerEvent *event)
{
    Q_UNUSED(event);
}

void KisToolPencil::mouseDoubleClickEvent(KoPointerEvent *event)
{
    Q_UNUSED(event)
}

void KisToolPencil::beginPrimaryAction(KoPointerEvent *event)
{
    if (!nodeEditable()) return;

    if (nodePaintAbility() == KisToolPencil::MYPAINTBRUSH_UNPAINTABLE) {
        KisCanvas2 * kiscanvas = static_cast<KisCanvas2*>(canvas());
        QString message = i18n("The MyPaint Brush Engine is not available for this colorspace");
        kiscanvas->viewManager()->showFloatingMessage(message, koIcon("object-locked"));
        event->ignore();
        return;
    }

    DelegatedPencilTool::mousePressEvent(event);
}

void KisToolPencil::continuePrimaryAction(KoPointerEvent *event)
{
    mouseMoveEvent(event);
}

void KisToolPencil::endPrimaryAction(KoPointerEvent *event)
{
    mouseReleaseEvent(event);
}

QList<QPointer<QWidget> > KisToolPencil::createOptionWidgets()
{
    QList<QPointer<QWidget> > widgetsList =
            DelegatedPencilTool::createOptionWidgets();

    QList<QPointer<QWidget> > filteredWidgets;
    Q_FOREACH (QWidget* widget, widgetsList) {
        if (widget->objectName() != "Stroke widget") {
            filteredWidgets.push_back(widget);
        }
    }
    return filteredWidgets;
}

__KisToolPencilLocalTool::__KisToolPencilLocalTool(KoCanvasBase * canvas, KisToolPencil* parentTool)
    : KoPencilTool(canvas), m_parentTool(parentTool) {
    setIsOpacityPresetMode(true);
}

void __KisToolPencilLocalTool::paint(QPainter &painter, const KoViewConverter &converter)
{
    if (m_parentTool->strokeStyle() == KisToolShapeUtils::StrokeStyleNone) {
        paintPath(path(), painter, converter);
    } else {
        KoPencilTool::paint(painter, converter);
    }
}



void __KisToolPencilLocalTool::paintPath(KoPathShape *pathShape, QPainter &painter, const KoViewConverter &converter)
{
    Q_UNUSED(converter);
    if (!pathShape) {
        return;
    }

    QTransform matrix;
    matrix.scale(m_parentTool->image()->xRes(), m_parentTool->image()->yRes());
    matrix.translate(pathShape->position().x(), pathShape->position().y());
    m_parentTool->paintToolOutline(&painter, m_parentTool->pixelToView(matrix.map(pathShape->outline())));
}

void __KisToolPencilLocalTool::addPathShape(KoPathShape* pathShape, bool closePath)
{
    if (closePath) {
        pathShape->close();
        pathShape->normalize();
    }

    m_parentTool->addPathShape(pathShape, kundo2_i18n("Draw Freehand Path"));
}

void __KisToolPencilLocalTool::slotUpdatePencilCursor()
{
    KoShapeStrokeSP stroke = this->createStroke();
    m_parentTool->updatePencilCursor(stroke && stroke->isVisible());

    auto style = m_parentTool->strokeStyle();
    if (style ==  KisToolShapeUtils::StrokeStyleForeground )
    {
        KoPencilTool::setStrokeColor(canvas()->resourceManager()->foregroundColor().toQColor());
    }
    else if ( style == KisToolShapeUtils::StrokeStyleBackground)
    {
        KoPencilTool::setStrokeColor(canvas()->resourceManager()->backgroundColor().toQColor());
    }
}
