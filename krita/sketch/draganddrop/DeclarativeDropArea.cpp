/*
    Copyright (C) 2010 by BetterInbox <contact@betterinbox.com>
    Original author: Gregory Schlomoff <greg@betterinbox.com>

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
*/

#include "DeclarativeDropArea.h"
#include "DeclarativeDragDropEvent.h"

#include <QMimeData>

DeclarativeDropArea::DeclarativeDropArea(QQuickItem *parent)
    : QQuickItem(parent),
      m_enabled(true),
      m_preventStealing(false),
      m_temporaryInhibition(false),
      m_containsDrag(false)
{
    setFlag(ItemAcceptsDrops, m_enabled);
    setFlag(ItemHasContents, m_enabled);
    setAcceptHoverEvents(m_enabled);

}

void DeclarativeDropArea::temporaryInhibitParent(bool inhibit)
{
    QQuickItem *candidate = parentItem();

    while (candidate) {
        if (DeclarativeDropArea *da = qobject_cast<DeclarativeDropArea *>(candidate)) {
            da->m_temporaryInhibition = inhibit;
            if (inhibit) {
                emit da->dragLeaveEvent(0);
            }
        }
        candidate = candidate->parentItem();
    }
}

void DeclarativeDropArea::dragEnterEvent(QDragEnterEvent *event)
{
    if (!m_enabled || m_temporaryInhibition) {
        return;
    }

    DeclarativeDragDropEvent dde(event, this);
    event->accept();

    if (m_preventStealing) {
        temporaryInhibitParent(true);
    }

    m_oldDragMovePos = event->pos();

    emit dragEnter(&dde);
    setContainsDrag(true);
}

void DeclarativeDropArea::dragLeaveEvent(QDragLeaveEvent *event)
{
    //do it anyways, in the unlikely case m_preventStealing
    //was changed while drag
    temporaryInhibitParent(false);

    m_oldDragMovePos = QPoint(-1,-1);
    DeclarativeDragDropEvent dde(event, this);
    emit dragLeave(&dde);
    setContainsDrag(false);
}

void DeclarativeDropArea::dragMoveEvent(QDragMoveEvent *event)
{
    if (!m_enabled || m_temporaryInhibition) {
        return;
    }

    //if the position we export didn't change, don't generate the move event
    if (event->pos() == m_oldDragMovePos) {
        event->setAccepted(false);
        return;
    }

    m_oldDragMovePos = event->pos();
    DeclarativeDragDropEvent dde(event, this);
    event->accept();
    emit dragMove(&dde);
}

void DeclarativeDropArea::dropEvent(QDropEvent *event)
{
    //do it anyways, in the unlikely case m_preventStealing
    //was changed while drag, do it after a loop,
    //so the parent dropevent doesn't get delivered
    metaObject()->invokeMethod(this, "temporaryInhibitParent", Qt::QueuedConnection, Q_ARG(bool, false));

    m_oldDragMovePos = QPoint(-1,-1);
    
    if (!m_enabled || m_temporaryInhibition) {
        return;
    }

    DeclarativeDragDropEvent dde(event, this);
    emit drop(&dde);
    setContainsDrag(false);
}

bool DeclarativeDropArea::isEnabled() const
{
    return m_enabled;
}

void DeclarativeDropArea::setEnabled(bool enabled)
{
    if (enabled == m_enabled) {
        return;
    }

    m_enabled = enabled;
    setAcceptHoverEvents(m_enabled);
    setFlag(ItemAcceptsDrops, m_enabled);
    emit enabledChanged();
}

bool DeclarativeDropArea::preventStealing() const
{
    return m_preventStealing;
}

void DeclarativeDropArea::setPreventStealing(bool prevent)
{
    if (prevent == m_preventStealing) {
        return;
    }

    m_preventStealing = prevent;
    emit preventStealingChanged();
}

void DeclarativeDropArea::setContainsDrag(bool dragging)
{
    if (m_containsDrag != dragging) {
        m_containsDrag = dragging;
        Q_EMIT containsDragChanged(m_containsDrag);
    }
}

bool DeclarativeDropArea::containsDrag() const
{
    return m_containsDrag;
}

