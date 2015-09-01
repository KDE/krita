/* This file is part of the KDE project
 * Copyright (C) 2010 Carlos Licea <carlos@kdab.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "CommentTool.h"
#include "CommentShape.h"
#include "InitialsCommentShape.h"

#include <KoShape.h>
#include <KoCanvasBase.h>
#include <KoShapeManager.h>

#include <QSet>
#include <KoPointerEvent.h>

CommentTool::CommentTool(KoCanvasBase* canvas)
: KoToolBase(canvas)
, m_canvas(canvas)
, m_previouseActiveCommentShape(0)
{
}

CommentTool::~CommentTool()
{
}

void CommentTool::activate(KoToolBase::ToolActivation toolActivation, const QSet< KoShape* >& /*shapes*/)
{
    const QCursor cursor(Qt::ArrowCursor);
    emit useCursor(cursor);

    QList<KoShape*> allShapes = m_canvas->shapeManager()->shapes();
    foreach( KoShape* shape, allShapes ) {
        InitialsCommentShape* initialsShape = dynamic_cast<InitialsCommentShape*>(shape);
        if(initialsShape) {
            initialsShape->setSelectable(true);
        }
    }

    m_temporary = toolActivation == KoToolBase::TemporaryActivation;
}

void CommentTool::deactivate()
{
    QList<KoShape*> allShapes = m_canvas->shapeManager()->shapes();
    foreach( KoShape* shape, allShapes ) {
        InitialsCommentShape* initialsShape = dynamic_cast<InitialsCommentShape*>(shape);
        if(initialsShape) {
            initialsShape->setSelectable(false);
        }
    }

    if(m_previouseActiveCommentShape) {
        m_previouseActiveCommentShape->toogleActive();
        m_previouseActiveCommentShape = 0;
    }
}


void CommentTool::mouseReleaseEvent(KoPointerEvent* event)
{
    //disable the previous activeshape
    if(m_previouseActiveCommentShape) {
        m_previouseActiveCommentShape->setActive(false);
    }

    InitialsCommentShape* initialsUnderCursor = dynamic_cast<InitialsCommentShape*>(m_canvas->shapeManager()->shapeAt(event->point, KoFlake::ShapeOnTop, false));
    if(initialsUnderCursor) {
        //don't re-activate the shape we just deactivated
        if(m_previouseActiveCommentShape == initialsUnderCursor->parent()) {
            m_previouseActiveCommentShape = 0;
            return;
        }

        CommentShape* commentUnderCursor = dynamic_cast<CommentShape*>(initialsUnderCursor->parent());
        Q_ASSERT(commentUnderCursor);
        commentUnderCursor->setActive(true);

        m_previouseActiveCommentShape = commentUnderCursor;
    }
    event->accept();
}

void CommentTool::mouseMoveEvent(KoPointerEvent* event)
{
    InitialsCommentShape* shapeUnderCursor = dynamic_cast<InitialsCommentShape*>(m_canvas->shapeManager()->shapeAt(event->point, KoFlake::ShapeOnTop, false));
    if(shapeUnderCursor) {
        const QCursor cursor(Qt::PointingHandCursor);
        emit useCursor(cursor);
    }
    else {
        const QCursor cursor(Qt::ArrowCursor);
        emit useCursor(cursor);
    }
}

void CommentTool::mousePressEvent(KoPointerEvent* /*event*/)
{
}

void CommentTool::paint(QPainter& /*painter*/, const KoViewConverter& /*converter*/)
{
}
