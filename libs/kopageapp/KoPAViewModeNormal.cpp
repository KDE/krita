/* This file is part of the KDE project
 * Copyright (  C ) 2007 Thorsten Zachmann <zachmann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (  at your option ) any later version.
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

#include "KoPAViewModeNormal.h"

#include <QEvent>
#include <QKeyEvent>

#include <KoToolProxy.h>

KoPAViewModeNormal::KoPAViewModeNormal( KoPAView * view, KoPACanvas * canvas )
: KoPAViewMode( view, canvas )
{
}

KoPAViewModeNormal::~KoPAViewModeNormal()
{
}

void KoPAViewModeNormal::paintEvent( QPaintEvent* event )
{
}

void KoPAViewModeNormal::tabletEvent( QTabletEvent *event, const QPointF &point )
{
    m_toolProxy->tabletEvent( event, point );
}

void KoPAViewModeNormal::mousePressEvent( QMouseEvent *event, const QPointF &point )
{
    m_toolProxy->mousePressEvent( event, point );
}

void KoPAViewModeNormal::mouseDoubleClickEvent( QMouseEvent *event, const QPointF &point )
{
    m_toolProxy->mouseDoubleClickEvent( event, point );
}

void KoPAViewModeNormal::mouseMoveEvent( QMouseEvent *event, const QPointF &point )
{
    m_toolProxy->mouseMoveEvent( event, point );
}

void KoPAViewModeNormal::mouseReleaseEvent( QMouseEvent *event, const QPointF &point )
{
    m_toolProxy->mouseReleaseEvent( event, point );
}

void KoPAViewModeNormal::keyPressEvent( QKeyEvent *event )
{
    m_toolProxy->keyPressEvent( event );
}

void KoPAViewModeNormal::keyReleaseEvent( QKeyEvent *event )
{
    m_toolProxy->keyReleaseEvent( event );
}

void KoPAViewModeNormal::wheelEvent( QWheelEvent * event, const QPointF &point )
{
    m_toolProxy->wheelEvent( event, point );
}
