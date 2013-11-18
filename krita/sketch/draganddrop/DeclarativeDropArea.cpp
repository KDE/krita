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

#include <QGraphicsSceneDragDropEvent>
#include <QMimeData>

DeclarativeDropArea::DeclarativeDropArea(QDeclarativeItem *parent)
    : QDeclarativeItem(parent),
    m_enabled(true)
{
    setAcceptDrops(m_enabled);
}

void DeclarativeDropArea::dragEnterEvent(QGraphicsSceneDragDropEvent *event) {
    DeclarativeDragDropEvent dde(event, this);
    emit dragEnter(&dde);
}

void DeclarativeDropArea::dragLeaveEvent(QGraphicsSceneDragDropEvent *event)
{
    DeclarativeDragDropEvent dde(event, this);
    emit dragLeave(&dde);
}

void DeclarativeDropArea::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    DeclarativeDragDropEvent dde(event, this);
    emit drop(&dde);
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
    setAcceptDrops(m_enabled);
    emit enabledChanged();
}
