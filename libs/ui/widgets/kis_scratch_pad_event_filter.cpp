/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
