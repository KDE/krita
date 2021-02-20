/*
    SPDX-FileCopyrightText: 2010 BetterInbox <contact@betterinbox.com>
    Original author: Gregory Schlomoff <greg@betterinbox.com>

    SPDX-License-Identifier: MIT
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

