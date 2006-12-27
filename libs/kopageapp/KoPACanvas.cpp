/* This file is part of the KDE project
   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>

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

#include "KoPACanvas.h"

#include <KoShapeManager.h>
#include <KoToolManager.h>
#include <KoToolProxy.h>
#include <KoUnit.h>

#include "KoPADocument.h"
#include "KoPAView.h"
#include "KoPAPage.h"

#include <QDebug>

KoPACanvas::KoPACanvas( KoPAView * view, KoPADocument * doc )
: QWidget( view )
, KoCanvasBase( doc )
, m_view( view )
, m_doc( doc )
{
    m_shapeManager = new KoShapeManager( this );
    m_toolProxy = KoToolManager::instance()->createToolProxy( this );
    setFocusPolicy( Qt::StrongFocus );
    // this is much faster than painting it in the paintevent
    setBackgroundRole( QPalette::Base );
    setAutoFillBackground( true );
    updateSize();
}

KoPACanvas::~KoPACanvas()
{
    delete m_toolProxy;
    delete m_shapeManager;
}

void KoPACanvas::updateSize()
{
  int width = 0;
  int height = 0;

  if ( m_view->activePage() ) 
  {
    KoPageLayout pageLayout = m_view->activePage()->pageLayout();
    width = qRound( m_view->zoomHandler()->zoomItX( pageLayout.ptWidth ) );
    height = qRound( m_view->zoomHandler()->zoomItX( pageLayout.ptHeight ) );
  }

  setMinimumSize( width, height );
}


void KoPACanvas::gridSize( double *horizontal, double *vertical ) const
{
    *horizontal = m_doc->gridData().gridX();
    *vertical = m_doc->gridData().gridY();
}

bool KoPACanvas::snapToGrid() const
{
    return m_doc->gridData().snapToGrid();
}

void KoPACanvas::addCommand( QUndoCommand *command )
{
    m_doc->addCommand( command );
}

KoShapeManager * KoPACanvas::shapeManager() const
{
    return m_shapeManager;
}

void KoPACanvas::updateCanvas( const QRectF& rc )
{
    QRect clipRect( viewConverter()->documentToView( rc ).toRect() );
    clipRect.adjust( -2, -2, 2, 2 ); // Resize to fit anti-aliasing
    update( clipRect );
}

KoViewConverter * KoPACanvas::viewConverter()
{
    return m_view->viewConverter();
}

KoUnit KoPACanvas::unit()
{
    return m_doc->unit();
}

void KoPACanvas::paintEvent( QPaintEvent *event )
{
    QPainter painter( this );
    painter.setRenderHint( QPainter::Antialiasing );
    painter.setClipRect( event->rect() );

    m_shapeManager->paint( painter, *( viewConverter() ), false );
    m_toolProxy->paint( painter, *( viewConverter() ) );
}

void KoPACanvas::tabletEvent( QTabletEvent *event )
{
    m_toolProxy->tabletEvent( event, viewConverter()->viewToDocument( event->pos() ) );
}

void KoPACanvas::mousePressEvent( QMouseEvent *event )
{
    m_toolProxy->mousePressEvent( event, viewConverter()->viewToDocument( event->pos() ) );
}

void KoPACanvas::mouseDoubleClickEvent( QMouseEvent *event )
{
    m_toolProxy->mouseDoubleClickEvent( event, viewConverter()->viewToDocument( event->pos() ) );
}

void KoPACanvas::mouseMoveEvent( QMouseEvent *event )
{
    m_toolProxy->mouseMoveEvent( event, viewConverter()->viewToDocument( event->pos() ) );
}

void KoPACanvas::mouseReleaseEvent( QMouseEvent *event )
{
    m_toolProxy->mouseReleaseEvent( event, viewConverter()->viewToDocument( event->pos() ) );
}

void KoPACanvas::keyPressEvent( QKeyEvent *event )
{
    m_toolProxy->keyPressEvent( event );
}

void KoPACanvas::keyReleaseEvent( QKeyEvent *event )
{
    m_toolProxy->keyReleaseEvent( event );
}

void KoPACanvas::wheelEvent ( QWheelEvent * event )
{
    m_toolProxy->wheelEvent( event, viewConverter()->viewToDocument( event->pos() ) );
}

#include "KoPACanvas.moc"
