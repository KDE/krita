/* This file is part of the KDE project

   Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoInteractionTool.h"
#include "KoPointerEvent.h"
#include "KoInteractionStrategy.h"
#include "KoCanvasBase.h"
#include "KoPanTool.h"

class KoInteractionTool::Private
{
public:
    Private()  { }

    QPointF lastPoint;
};

KoInteractionTool::KoInteractionTool(KoCanvasBase *canvas)
        : KoTool(canvas),
        m_currentStrategy(0),
        d(new Private())
{
}

KoInteractionTool::~KoInteractionTool()
{
    delete d;
    delete m_currentStrategy;
    m_currentStrategy = 0;
}

void KoInteractionTool::paint(QPainter &painter, const KoViewConverter &converter)
{
    if (m_currentStrategy)
        m_currentStrategy->paint(painter, converter);
}

void KoInteractionTool::mousePressEvent(KoPointerEvent *event)
{
    if (m_currentStrategy) { // possible if the user presses an extra mouse button
        m_currentStrategy->cancelInteraction();
        delete m_currentStrategy;
        m_currentStrategy = 0;
        return;
    }
    m_currentStrategy = createStrategy(event);
    if (m_currentStrategy == 0)
        event->ignore();
}

void KoInteractionTool::mouseMoveEvent(KoPointerEvent *event)
{
    d->lastPoint = event->point;
    if (m_currentStrategy) {
        m_currentStrategy->handleMouseMove(d->lastPoint, event->modifiers());
    } else
        event->ignore();
}

void KoInteractionTool::mouseReleaseEvent(KoPointerEvent *event)
{
    if (m_currentStrategy) {
        m_currentStrategy->finishInteraction(event->modifiers());
        QUndoCommand *command = m_currentStrategy->createCommand();
        if (command)
            m_canvas->addCommand(command);
        delete m_currentStrategy;
        m_currentStrategy = 0;
        repaintDecorations();
    } else
        event->ignore();
}

void KoInteractionTool::keyPressEvent(QKeyEvent *event)
{
    event->ignore();
    if (m_currentStrategy &&
            (event->key() == Qt::Key_Control ||
             event->key() == Qt::Key_Alt || event->key() == Qt::Key_Shift ||
             event->key() == Qt::Key_Meta)) {
        m_currentStrategy->handleMouseMove(d->lastPoint, event->modifiers());
        event->accept();
    }
}

void KoInteractionTool::keyReleaseEvent(QKeyEvent *event)
{
    if (m_currentStrategy == 0) { // catch all cases where no current strategy is needed
        if (event->key() == Qt::Key_Space)
            emit activateTemporary(KoPanTool_ID);
    } else if (event->key() == Qt::Key_Escape) {
        m_currentStrategy->cancelInteraction();
        delete m_currentStrategy;
        m_currentStrategy = 0;
        event->accept();
    } else if (event->key() == Qt::Key_Control ||
               event->key() == Qt::Key_Alt || event->key() == Qt::Key_Shift ||
               event->key() == Qt::Key_Meta) {
        m_currentStrategy->handleMouseMove(d->lastPoint, event->modifiers());
    }
}

#include <KoInteractionTool.moc>
