/* This file is part of the KDE project
 * Copyright ( C ) 2007 Thorsten Zachmann <zachmann@kde.org>
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
#include <QPainter>

#include <KoToolProxy.h>
#include <KoShapeManager.h>
#include "KoPACanvas.h"
#include "KoPADocument.h"
#include "KoPAPage.h"
#include "KoPAMasterPage.h"
#include "KoPAView.h"

KoPAViewModeNormal::KoPAViewModeNormal( KoPAView * view, KoPACanvas * canvas )
: KoPAViewMode( view, canvas )
, m_masterMode( false )
, m_savedPage( 0 )
{
}

KoPAViewModeNormal::~KoPAViewModeNormal()
{
}

void KoPAViewModeNormal::paintEvent( QPaintEvent* event )
{
    QPainter painter( m_canvas );
    painter.translate( -m_canvas->documentOffset() );
    painter.setRenderHint( QPainter::Antialiasing );
    QRectF clipRect = event->rect().translated( m_canvas->documentOffset() );
    painter.setClipRect( clipRect );

    paintGrid( painter, *(m_view->viewConverter()), clipRect );
    m_canvas->masterShapeManager()->paint( painter, *( m_view->viewConverter() ), false );
    m_canvas->shapeManager()->paint( painter, *( m_view->viewConverter() ), false );
    m_toolProxy->paint( painter, *( m_view->viewConverter() ) );
}

void KoPAViewModeNormal::paintGrid(QPainter &painter, const KoViewConverter &converter, const QRectF &area)
{
    if( ! m_canvas->document()->gridData().showGrid() )
        return;

    painter.setPen( m_canvas->document()->gridData().gridColor() );

    double gridX = m_canvas->document()->gridData().gridX();

    double x = 0.0;
    do {
        painter.drawLine( converter.documentToView( QPointF( x, area.top() ) ),
                          converter.documentToView( QPointF( x, area.bottom() ) ) );
        x += gridX;
    } while( x <= area.right() );

    x = - gridX;
    while( x >= area.left() )
    {
        painter.drawLine( converter.documentToView( QPointF( x, area.top() ) ),
                          converter.documentToView( QPointF( x, area.bottom() ) ) );
        x -= gridX;
    };

    double gridY = m_canvas->document()->gridData().gridY();

    double y = 0.0;
    do {
        painter.drawLine( converter.documentToView( QPointF( area.left(), y ) ),
                          converter.documentToView( QPointF( area.right(), y ) ) );
        y += gridY;
    } while( y <= area.bottom() );

    y = - gridY;
    while( y >= area.top() )
    {
        painter.drawLine( converter.documentToView( QPointF( area.left(), y ) ),
                          converter.documentToView( QPointF( area.right(), y ) ) );
        y -= gridY;
    };
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

    if ( ! event->isAccepted() ) {
        event->accept();

        switch ( event->key() )
        {
            case Qt::Key_Home:
                m_view->navigatePage( KoPageApp::PageFirst );
                break;
            case Qt::Key_PageUp:
                m_view->navigatePage( KoPageApp::PagePrevious );
                break;
            case Qt::Key_PageDown:
                m_view->navigatePage( KoPageApp::PageNext );
                break;
            case Qt::Key_End:
                m_view->navigatePage( KoPageApp::PageLast );
                break;
            default:
                event->ignore();
                break;
        }
    }
}

void KoPAViewModeNormal::keyReleaseEvent( QKeyEvent *event )
{
    m_toolProxy->keyReleaseEvent( event );
}

void KoPAViewModeNormal::wheelEvent( QWheelEvent * event, const QPointF &point )
{
    m_toolProxy->wheelEvent( event, point );
}

void KoPAViewModeNormal::setMasterMode( bool master )
{
    m_masterMode = master;
    KoPAPage * page = dynamic_cast<KoPAPage *>( m_view->activePage() );
    if ( m_masterMode ) {
        if ( page ) {
            m_view->updateActivePage( page->masterPage() );
            m_savedPage = page;
        }
    }
    else if ( m_savedPage ) {
        m_view->updateActivePage( m_savedPage );
        m_savedPage = 0;
    }
}

bool KoPAViewModeNormal::masterMode()
{
    return m_masterMode;
}
