/* This file is part of the KDE project
   Copyright (C) 2006-2007 Thorsten Zachmann <zachmann@kde.org>

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
#include <KoToolProxy.h>
#include <KoUnit.h>

#include "KoPADocument.h"
#include "KoPAView.h"
#include "KoPAViewMode.h"
#include "KoPAPage.h"

#include <QDebug>

KoPACanvas::KoPACanvas( KoPAView * view, KoPADocument * doc )
: QWidget( view )
, KoCanvasBase( doc )
, m_view( view )
, m_doc( doc )
{
    m_shapeManager = new KoShapeManager( this );
    m_masterShapeManager = new KoShapeManager( this );
    m_toolProxy = new KoToolProxy( this );
    setFocusPolicy( Qt::StrongFocus );
    // this is much faster than painting it in the paintevent
    setBackgroundRole( QPalette::Base );
    setAutoFillBackground( true );
    updateSize();
}

KoPACanvas::~KoPACanvas()
{
    delete m_toolProxy;
    delete m_masterShapeManager;
    delete m_shapeManager;
}

void KoPACanvas::updateSize()
{
    QSize size;

    if ( m_view->activePage() ) 
    {
        KoPageLayout pageLayout = m_view->activePage()->pageLayout();
        size.setWidth( qRound( m_view->zoomHandler()->zoomItX( pageLayout.width ) ) );
        size.setHeight( qRound( m_view->zoomHandler()->zoomItX( pageLayout.height ) ) );
    }

    emit documentSize(size);
}

void KoPACanvas::setDocumentOffset(const QPoint &offset) {
    m_documentOffset = offset;
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

KoShapeManager * KoPACanvas::masterShapeManager() const
{
    return m_masterShapeManager;
}

void KoPACanvas::updateCanvas( const QRectF& rc )
{
    QRect clipRect( viewConverter()->documentToView( rc ).toRect() );
    clipRect.adjust( -2, -2, 2, 2 ); // Resize to fit anti-aliasing
    clipRect.moveTopLeft( clipRect.topLeft() - m_documentOffset);
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

const QPoint & KoPACanvas::documentOffset() const
{
    return m_documentOffset;
}

void KoPACanvas::paintEvent( QPaintEvent *event )
{
    m_view->viewMode()->paintEvent( event );
}

void KoPACanvas::tabletEvent( QTabletEvent *event )
{
    m_view->viewMode()->tabletEvent( event, viewConverter()->viewToDocument( event->pos() + m_documentOffset ) );
}

void KoPACanvas::mousePressEvent( QMouseEvent *event )
{
    m_view->viewMode()->mousePressEvent( event, viewConverter()->viewToDocument( event->pos() + m_documentOffset ) );
}

void KoPACanvas::mouseDoubleClickEvent( QMouseEvent *event )
{
    m_view->viewMode()->mouseDoubleClickEvent( event, viewConverter()->viewToDocument( event->pos() + m_documentOffset ) );
}

void KoPACanvas::mouseMoveEvent( QMouseEvent *event )
{
    m_view->viewMode()->mouseMoveEvent( event, viewConverter()->viewToDocument( event->pos() + m_documentOffset ) );
}

void KoPACanvas::mouseReleaseEvent( QMouseEvent *event )
{
    m_view->viewMode()->mouseReleaseEvent( event, viewConverter()->viewToDocument( event->pos() + m_documentOffset ) );
}

void KoPACanvas::keyPressEvent( QKeyEvent *event )
{
    m_view->viewMode()->keyPressEvent( event );
}

void KoPACanvas::keyReleaseEvent( QKeyEvent *event )
{
    m_view->viewMode()->keyReleaseEvent( event );
}

void KoPACanvas::wheelEvent ( QWheelEvent * event )
{
    m_view->viewMode()->wheelEvent( event, viewConverter()->viewToDocument( event->pos() + m_documentOffset ) );
}

bool KoPACanvas::event (QEvent *event) {
    // we should forward tabs, and let tools decide if they should be used or ignored.
    // if the tool ignores it, it will move focus.
    if(event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*> (event);
        if(keyEvent->key() == Qt::Key_Backtab)
            return true;
        if(keyEvent->key() == Qt::Key_Tab && event->type() == QEvent::KeyPress) {
            // we loose key-release events, which I think is not an issue.
            keyPressEvent(keyEvent);
            return true;
        }
    }
    return QWidget::event(event);
}

#include "KoPACanvas.moc"
