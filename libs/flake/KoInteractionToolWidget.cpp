/* This file is part of the KDE project
 * Copyright (C) 2007 Martin Pfeiffer <hubipete@gmx.net>
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

#include "KoInteractionToolWidget.h"
#include "KoInteractionTool.h"
#include <QLabel>
#include <QPainter>
#include <QSize>

KoInteractionToolWidget::KoInteractionToolWidget( KoInteractionTool* tool,
                                    QWidget* parent ) : QTabWidget( parent )
{
    m_tool = tool;

    setupUi( this );
    rectLabel->installEventFilter( this );

    bringToFront->setDefaultAction( m_tool->action( "object_move_totop" ) );
    raiseLevel->setDefaultAction( m_tool->action( "object_move_up" ) );
    lowerLevel->setDefaultAction( m_tool->action( "object_move_down" ) );
    sendBack->setDefaultAction( m_tool->action( "object_move_up" ) );
    bottomAlign->setDefaultAction( m_tool->action( "object_align_vertical_bottom" ) );
    vCenterAlign->setDefaultAction( m_tool->action( "object_align_vertical_center" ) );
    topAlign->setDefaultAction( m_tool->action( "object_align_vertical_top" ) );
    rightAlign->setDefaultAction( m_tool->action( "object_align_horizontal_right" ) );
    hCenterAlign->setDefaultAction( m_tool->action( "object_align_horizontal_center" ) );
    leftAlign->setDefaultAction( m_tool->action( "object_align_horizontal_left" ) );
}

bool KoInteractionToolWidget::eventFilter( QObject* object, QEvent* event )
{
    if( event->type() == QEvent::MouseButtonPress ) {
        return true;
    }
    else if( event->type() == QEvent::Paint ) {
        QPainter p( rectLabel );
        QRect r( 5, 5, rectLabel->width()-10, rectLabel->height()-10 );
        p.drawRect( r );
        p.drawRect( QRect( r.topLeft() - QPoint( 2, 2 ), QSize( 4, 4 ) ) );
        p.drawRect( QRect( r.topRight() - QPoint( 2, 2 ), QSize( 4, 4 ) ) );
        p.drawRect( QRect( r.bottomLeft() - QPoint( 2, 2 ), QSize( 4, 4 ) ) );
        p.drawRect( QRect( r.bottomRight() - QPoint( 2, 2 ), QSize( 4, 4 ) ) );
        return true;
    }
    else
        return QObject::eventFilter( object, event ); // standart event processing
}

#include <KoInteractionToolWidget.moc>
