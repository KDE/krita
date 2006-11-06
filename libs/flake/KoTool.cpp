/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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

#include <QWidget>
#include <QLabel>

#include <klocale.h>
#include <kdebug.h>

#include "KoTool.h"
#include "KoCanvasBase.h"
#include "KoViewConverter.h"
#include "KoPointerEvent.h"
#include "KoCanvasResourceProvider.h"

KoTool::KoTool(KoCanvasBase *canvas )
    : m_canvas(canvas)
    , m_optionWidget( 0 )
    , m_previousCursor(Qt::ArrowCursor)
{
    if(m_canvas) { 
        KoCanvasResourceProvider * crp = m_canvas->resourceProvider();
        Q_ASSERT_X(crp, "KoTool::KoTool", "No KoCanvasResourceProvider");
        if (crp)
            connect( m_canvas->resourceProvider(),
                 SIGNAL( sigResourceChanged(const KoCanvasResource & ) ),
                 this,
                 SLOT( resourceChanged( const KoCanvasResource &  ) ) );
    }
}

void KoTool::activate(bool temporary) {
    Q_UNUSED(temporary);
}

void KoTool::deactivate() {
}

void KoTool::resourceChanged( const KoCanvasResource & res )
{
    Q_UNUSED( res );
}

bool KoTool::wantsAutoScroll() {
    return true;
}

void KoTool::mouseDoubleClickEvent( KoPointerEvent *event ) {
    event->ignore();
}

void KoTool::keyPressEvent(QKeyEvent *e) {
    e->ignore();
}

void KoTool::keyReleaseEvent(QKeyEvent *e) {
    e->ignore();
}

void KoTool::wheelEvent( KoPointerEvent * e ) {
    e->ignore();
}


void KoTool::useCursor(QCursor cursor, bool force) {
    if(!force && cursor.shape() == m_previousCursor.shape())
        return;
    m_previousCursor = cursor;
    emit sigCursorChanged(m_previousCursor);
}

QWidget * KoTool::optionWidget(QWidget * parent) {
    // Create the optionwidget if it doesn't exist yet
    if (m_optionWidget == 0) {
        createOptionWidget(parent);
    }
    // If there is an optionwidget, but it is owned by a different widget,
    // reparent. Note: is setParent correct here, or should I use reParent?
    // The Qt dox are not conclusive.
    if (m_optionWidget != 0 && m_optionWidget->parent() != parent) {
        m_optionWidget->setParent(parent);
    }
    return m_optionWidget;
}

#include "KoTool.moc"
