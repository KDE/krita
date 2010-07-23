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

#include <KoShape.h>
#include <KoCanvasBase.h>
#include <KoShapeManager.h>

#include <QSet>
#include <KoPointerEvent.h>

CommentTool::CommentTool(KoCanvasBase* canvas)
: KoToolBase(canvas)
, m_canvas(canvas)
, m_previouseActiveShape(0)
{
}

CommentTool::~CommentTool()
{
}

void CommentTool::activate(KoToolBase::ToolActivation toolActivation, const QSet< KoShape* >& shapes)
{
    m_temporary = toolActivation == KoToolBase::TemporaryActivation;
}

void CommentTool::deactivate()
{
    if(m_previouseActiveShape) {
        m_previouseActiveShape->toogleSelect();
        m_previouseActiveShape = 0;
    }
}


void CommentTool::mouseReleaseEvent(KoPointerEvent* event)
{
    CommentShape* shapeUnderCursor = dynamic_cast<CommentShape*>(m_canvas->shapeManager()->shapeAt(event->point, KoFlake::ShapeOnTop, false));
    if(m_previouseActiveShape != shapeUnderCursor) {
        if(m_previouseActiveShape) {
            m_previouseActiveShape->toogleSelect();
        }
        if(shapeUnderCursor) {
            shapeUnderCursor->toogleSelect();
        }
        m_previouseActiveShape = shapeUnderCursor;
    }
}

void CommentTool::mouseMoveEvent(KoPointerEvent* event)
{
}

void CommentTool::mousePressEvent(KoPointerEvent* event)
{
}

void CommentTool::paint(QPainter& painter, const KoViewConverter& converter)
{
}

