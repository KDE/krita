/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_scratch_pad_event_filter.h"
#include "kis_scratch_pad.h"
#include <QWidget>
#include <QDebug>


KisScratchPadEventFilter::KisScratchPadEventFilter(QWidget *parent)
    : QObject(parent),
      m_tabletPressed(false)
{
    parent->installEventFilter(this);
    m_scratchPad = qobject_cast<KisScratchPad *>(parent);
}

void KisScratchPadEventFilter::setWidgetToDocumentTransform(const QTransform &transform)
{
    m_widgetToDocument = transform;
}

QWidget* KisScratchPadEventFilter::parentWidget()
{
    return static_cast<QWidget*>(parent());
}

KoPointerEvent* KisScratchPadEventFilter::createMouseEvent(QEvent *event)
{
    QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
    return new KoPointerEvent(mouseEvent, m_widgetToDocument.map(mouseEvent->pos()));
}

KoPointerEvent* KisScratchPadEventFilter::createTabletEvent(QEvent *event)
{
    QTabletEvent *tabletEvent = static_cast<QTabletEvent*>(event);
    const QPointF pos = tabletEvent->posF();

    KoPointerEvent *ev = new KoPointerEvent(tabletEvent, m_widgetToDocument.map(pos));
    ev->setTabletButton(Qt::LeftButton);

    return ev;
}

bool KisScratchPadEventFilter::eventFilter(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj);
    bool result = true;

    KoPointerEvent *ev = 0;


    switch(event->type()) {
    case QEvent::MouseButtonPress:
        if(m_tabletPressed) break;
        ev = createMouseEvent(event);
        m_scratchPad->pointerPress(ev);
        break;
    case QEvent::MouseButtonRelease:
        if(m_tabletPressed) break;
        ev = createMouseEvent(event);
        m_scratchPad->pointerRelease(ev);
        break;
    case QEvent::MouseMove:
        if(m_tabletPressed) break;
        ev = createMouseEvent(event);
        m_scratchPad->pointerMove(ev);
        break;
    case QEvent::TabletPress:
        // if(m_tabletPressed) break;
        m_tabletPressed = true;
        ev = createTabletEvent(event);
        m_scratchPad->pointerPress(ev);
        break;
    case QEvent::TabletRelease:
        // if(!m_tabletPressed) break;
        m_tabletPressed = false;
        ev = createTabletEvent(event);
        m_scratchPad->pointerRelease(ev);
        break;
    case QEvent::TabletMove:
        // if(!m_tabletPressed) break;
        ev = createTabletEvent(event);
        m_scratchPad->pointerMove(ev);
        break;
    default:
        result = false;
    }

    if(ev) {
        result = ev->isAccepted();
        event->setAccepted(result);
        delete ev;
    }

    return result;
}
