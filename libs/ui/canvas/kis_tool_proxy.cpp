/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_tool_proxy.h"
#include "kis_canvas2.h"
#include "input/kis_tablet_debugger.h"

#include <KoToolProxy_p.h>


KisToolProxy::KisToolProxy(KoCanvasBase *canvas, QObject *parent)
    : KoToolProxy(canvas, parent),
      m_isActionActivated(false),
      m_lastAction(KisTool::Primary)
{
}

void KisToolProxy::initializeImage(KisImageSP image)
{
    connect(image, SIGNAL(sigUndoDuringStrokeRequested()), SLOT(requestUndoDuringStroke()), Qt::UniqueConnection);
    connect(image, SIGNAL(sigStrokeCancellationRequested()), SLOT(requestStrokeCancellation()), Qt::UniqueConnection);
    connect(image, SIGNAL(sigStrokeEndRequested()), SLOT(requestStrokeEnd()), Qt::UniqueConnection);
}

QPointF KisToolProxy::tabletToDocument(const QPointF &globalPos)
{
    const QPointF pos = globalPos - QPointF(canvas()->canvasWidget()->mapToGlobal(QPoint(0, 0)));
    return widgetToDocument(pos);
}

QPointF KisToolProxy::widgetToDocument(const QPointF &widgetPoint) const
{
    KisCanvas2 *kritaCanvas = dynamic_cast<KisCanvas2*>(canvas());
    Q_ASSERT(kritaCanvas);

    return kritaCanvas->coordinatesConverter()->widgetToDocument(widgetPoint);
}

KoPointerEvent KisToolProxy::convertEventToPointerEvent(QEvent *event, const QPointF &docPoint, bool *result)
{
    switch (event->type()) {
    case QEvent::TabletPress:
    case QEvent::TabletRelease:
    case QEvent::TabletMove:
    {
        *result = true;
        QTabletEvent *tabletEvent = static_cast<QTabletEvent*>(event);
        KoPointerEvent ev(tabletEvent, docPoint);
        ev.setTabletButton(Qt::LeftButton);
        return ev;
    }
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseMove:
    {
        *result = true;
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        return KoPointerEvent(mouseEvent, docPoint);
    }
    case QEvent::TouchBegin:
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd:
    {
        *result = true;
        QTouchEvent *touchEvent = static_cast<QTouchEvent *> (event);
        return KoPointerEvent(touchEvent, docPoint);
    }
    default:
        ;
    }

    *result = false;
    QMouseEvent fakeEvent(QEvent::MouseMove, QPoint(),
                          Qt::NoButton, Qt::NoButton,
                          Qt::NoModifier);

    return KoPointerEvent(&fakeEvent, QPointF());
}

void KisToolProxy::forwardHoverEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::TabletMove: {
        QTabletEvent *tabletEvent = static_cast<QTabletEvent*>(event);
        QPointF docPoint = widgetToDocument(tabletEvent->posF());
        this->tabletEvent(tabletEvent, docPoint);
        return;
    }

    case QEvent::MouseMove: {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        QPointF docPoint = widgetToDocument(mouseEvent->localPos());
        mouseMoveEvent(mouseEvent, docPoint);
        return;
    }

    default: {
        qWarning() << "forwardHoverEvent encountered unknown event type:"
            << event->type();
        return;
    }
    }
}

bool KisToolProxy::forwardEvent(ActionState state, KisTool::ToolAction action, QEvent *event, QEvent *originalEvent)
{
    bool retval = true;

    QTabletEvent *tabletEvent = dynamic_cast<QTabletEvent*>(event);
    QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent*>(event);
    QTouchEvent *touchEvent = dynamic_cast<QTouchEvent *> (event);

    if (tabletEvent) {
        QPointF docPoint = widgetToDocument(tabletEvent->posF());
        tabletEvent->accept();
        this->tabletEvent(tabletEvent, docPoint);
        forwardToTool(state, action, tabletEvent, docPoint);
        retval = tabletEvent->isAccepted();
    }
    else if (mouseEvent) {
        QPointF docPoint = widgetToDocument(mouseEvent->localPos());
        mouseEvent->accept();
        if (mouseEvent->type() == QEvent::MouseButtonPress) {
            mousePressEvent(mouseEvent, docPoint);
        } else if (mouseEvent->type() == QEvent::MouseButtonDblClick) {
            mouseDoubleClickEvent(mouseEvent, docPoint);
        } else if (mouseEvent->type() == QEvent::MouseButtonRelease) {
            mouseReleaseEvent(mouseEvent, docPoint);
        } else if (mouseEvent->type() == QEvent::MouseMove) {
            mouseMoveEvent(mouseEvent, docPoint);
        }
        forwardToTool(state, action, originalEvent, docPoint);
        retval = mouseEvent->isAccepted();
    }
    else if (touchEvent) {
        QPointF docPoint = widgetToDocument(touchEvent->touchPoints().at(0).pos());
        touchEvent->accept();
        this->touchEvent(touchEvent, docPoint);
        forwardToTool(state, action, touchEvent, docPoint);
        retval = touchEvent->isAccepted();
    }
    else if (event && event->type() == QEvent::KeyPress) {
        QKeyEvent* kevent = static_cast<QKeyEvent*>(event);
        keyPressEvent(kevent);
    }
    else if (event && event->type() == QEvent::KeyRelease) {
        QKeyEvent* kevent = static_cast<QKeyEvent*>(event);
        keyReleaseEvent(kevent);
    }

    return retval;
}

void KisToolProxy::forwardToTool(ActionState state, KisTool::ToolAction action, QEvent *event, const QPointF &docPoint)
{
    bool eventValid = false;
    KoPointerEvent ev = convertEventToPointerEvent(event, docPoint, &eventValid);
    KisTool *activeTool = dynamic_cast<KisTool*>(priv()->activeTool);

    if (!eventValid || !activeTool) return;

    switch (state) {
    case BEGIN:
        if (action == KisTool::Primary) {
            if (event->type() == QEvent::MouseButtonDblClick) {
                activeTool->beginPrimaryDoubleClickAction(&ev);
            } else {
                activeTool->beginPrimaryAction(&ev);
            }
        } else {
            if (event->type() == QEvent::MouseButtonDblClick) {
                activeTool->beginAlternateDoubleClickAction(&ev, KisTool::actionToAlternateAction(action));
            } else {
                activeTool->beginAlternateAction(&ev, KisTool::actionToAlternateAction(action));
            }
        }
        break;
    case CONTINUE:
        if (action == KisTool::Primary) {
            activeTool->continuePrimaryAction(&ev);
        } else {
            activeTool->continueAlternateAction(&ev, KisTool::actionToAlternateAction(action));
        }
        break;
    case END:
        if (action == KisTool::Primary) {
            activeTool->endPrimaryAction(&ev);
        } else {
            activeTool->endAlternateAction(&ev, KisTool::actionToAlternateAction(action));
        }
        break;
    }
}

bool KisToolProxy::primaryActionSupportsHiResEvents() const
{
    KisTool *activeTool = dynamic_cast<KisTool*>(const_cast<KisToolProxy*>(this)->priv()->activeTool);
    return activeTool && activeTool->primaryActionSupportsHiResEvents();
}

void KisToolProxy::setActiveTool(KoToolBase *tool)
{
    if (!tool) return;

    if (m_isActionActivated) {
        deactivateToolAction(m_lastAction);
        KoToolProxy::setActiveTool(tool);
        activateToolAction(m_lastAction);
    } else {
        KoToolProxy::setActiveTool(tool);
    }
}

void KisToolProxy::activateToolAction(KisTool::ToolAction action)
{
    if (!action) {
        return;
    }
    KisTool *activeTool = dynamic_cast<KisTool*>(const_cast<KisToolProxy*>(this)->priv()->activeTool);

    if (activeTool) {
        if (action == KisTool::Primary) {
            activeTool->activatePrimaryAction();
        } else {
            activeTool->activateAlternateAction(KisTool::actionToAlternateAction(action));
        }
    }

    m_isActionActivated = true;
    m_lastAction = action;
}

void KisToolProxy::deactivateToolAction(KisTool::ToolAction action)
{
    KisTool *activeTool = dynamic_cast<KisTool*>(const_cast<KisToolProxy*>(this)->priv()->activeTool);

    if (activeTool) {
        if (action == KisTool::Primary) {
            activeTool->deactivatePrimaryAction();
        } else {
            activeTool->deactivateAlternateAction(KisTool::actionToAlternateAction(action));
        }
    }

    m_isActionActivated = false;
    m_lastAction = action;
}
