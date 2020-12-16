/*
    SPDX-FileCopyrightText: 2010 BetterInbox <contact@betterinbox.com>
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>
    Original author: Gregory Schlomoff <greg@betterinbox.com>

    SPDX-License-Identifier: MIT
*/

#include "DeclarativeDragDropEvent.h"
#include "DeclarativeMimeData.h"

DeclarativeDragDropEvent::DeclarativeDragDropEvent(QDropEvent* e, DeclarativeDropArea* parent) :
    QObject(parent),
    m_x(e->pos().x()),
    m_y(e->pos().y()),
    m_buttons(e->mouseButtons()),
    m_modifiers(e->keyboardModifiers()),
    m_data(0),
    m_event(e)
{
}

DeclarativeDragDropEvent::DeclarativeDragDropEvent(QDragLeaveEvent* e, DeclarativeDropArea* parent) :
    QObject(parent),
    m_x(0),
    m_y(0),
    m_buttons(Qt::NoButton),
    m_modifiers(Qt::NoModifier),
    m_data(0),
    m_event(0)
{
    Q_UNUSED(e);
}

void DeclarativeDragDropEvent::accept(int action)
{
    m_event->setDropAction( (Qt::DropAction) action );
//     qDebug() << "-----> Accepting event: " << this << m_data.urls() << m_data.text() << m_data.html() << ( m_data.hasColor() ? m_data.color().name() : " no color");
    m_event->accept();
}

DeclarativeMimeData* DeclarativeDragDropEvent::mimeData()
{
    if (!m_data && m_event) {
//         TODO This should be using MimeDataWrapper eventually, although this is an API break,
//         so will need to be done carefully.
        m_data = new DeclarativeMimeData(m_event->mimeData());
    }
    return m_data;
}

