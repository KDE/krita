/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#include "KoPanTool.h"
#include "KoPointerEvent.h"
#include "KoCanvasBase.h"
#include "KoCanvasController.h"
#include "KoViewConverter.h"

#include <QKeyEvent>
#include <kdebug.h>

KoPanTool::KoPanTool(KoCanvasBase *canvas)
    : KoTool(canvas),
    m_controller(0),
    m_temporary(false)
{
}

bool KoPanTool::wantsAutoScroll() {
    return false;
}

void KoPanTool::mousePressEvent( KoPointerEvent *event ) {
    m_lastPosition = event->point;
    event->accept();
    useCursor(QCursor(Qt::ClosedHandCursor));
}

void KoPanTool::mouseMoveEvent( KoPointerEvent *event ) {
    Q_ASSERT(m_controller);
    if(event->buttons() == 0)
        return;
    event->accept();

    QPointF distance( m_canvas->viewConverter()->documentToView(m_lastPosition - event->point) );
    m_controller->pan(distance.toPoint());

    m_lastPosition = event->point;
}

void KoPanTool::mouseReleaseEvent( KoPointerEvent *event ) {
    event->accept();
    useCursor(QCursor(Qt::OpenHandCursor));
    if(m_temporary)
        emit sigDone();
}

void KoPanTool::keyPressEvent(QKeyEvent *event) {
    // TODO use arrow bottons to scroll.
    event->accept();
}

void KoPanTool::activate(bool temporary) {
    if(m_controller == 0)
        emit sigDone();
    m_temporary = temporary;
    useCursor(QCursor(Qt::OpenHandCursor), true);
}
