/*
    Copyright (C) 2010 by BetterInbox <contact@betterinbox.com>
    Copyright 2013 by Sebastian Kügler <sebas@kde.org>
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

