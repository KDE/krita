/* This file is part of the KDE project
   Copyright (C) 1998, 1999, 2000 Torben Weis <weis@kde.org>

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

#include "KoContainerHandler.h"
#include <KoView.h>
#include <math.h>
#include <kcursor.h>
#include <kdebug.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QKeyEvent>
#include <QEvent>

KoEventHandler::KoEventHandler( QObject* target )
{
    m_target = target;

    m_target->installEventFilter( this );
}

KoEventHandler::~KoEventHandler()
{
}

QObject* KoEventHandler::target()
{
    return m_target;
}

// ------------------------------------------------------

class KoPartResizeHandlerPrivate {
public:
    KoPartResizeHandlerPrivate( const QMatrix& matrix, KoView *view, KoChild* child,
                              KoChild::Gadget gadget, const QPoint& point ) :
        m_gadget(gadget), m_view(view), m_child(child), m_parentMatrix(matrix) {

        m_geometryStart = child->geometry();
        m_matrix = child->matrix() * matrix;
        m_invertParentMatrix = matrix.inverted();

        bool ok = true;
        m_invert = m_matrix.inverted( &ok );
        Q_ASSERT( ok );
        m_mouseStart = m_invert.map( m_invertParentMatrix.map( point ) );
    }
    ~KoPartResizeHandlerPrivate() {}

    KoChild::Gadget m_gadget;
    QPoint m_mouseStart;
    QRect m_geometryStart;
    KoView* m_view;
    KoChild* m_child;
    QMatrix m_invert;
    QMatrix m_matrix;
    QMatrix m_parentMatrix;
    QMatrix m_invertParentMatrix;
};

KoPartResizeHandler::KoPartResizeHandler( QWidget* widget, const QMatrix& matrix, KoView* view, KoChild* child,
                                      KoChild::Gadget gadget, const QPoint& point )
    : KoEventHandler( widget )
{
    child->lock();
    d=new KoPartResizeHandlerPrivate(matrix, view, child, gadget, point);
}

KoPartResizeHandler::~KoPartResizeHandler()
{
    d->m_child->unlock();
    delete d;
    d=0L;
}

void KoPartResizeHandler::repaint(QRegion &rgn)
{
  rgn = rgn.unite( d->m_child->frameRegion( d->m_parentMatrix, true ) );
 // rgn.translate(- d->m_view->canvasXOffset(), - d->m_view->canvasYOffset());
  ((QWidget*)target())->repaint( rgn );
}

bool KoPartResizeHandler::eventFilter( QObject*, QEvent* ev )
{
    if ( ev->type() == QEvent::MouseButtonRelease )
    {
        delete this;
        return true;
    }
    else if ( ev->type() == QEvent::MouseMove )
    {
        QMouseEvent* e = (QMouseEvent*)ev;
        QPoint p = d->m_invert.map( d->m_invertParentMatrix.map( e->pos() /*+ QPoint(d->m_view->canvasXOffset(), d->m_view->canvasYOffset()) */ ) );
        QRegion rgn( d->m_child->frameRegion( d->m_parentMatrix, true ) );

        double x1_x, x1_y, x2_x, x2_y;
        d->m_matrix.map( double( p.x() ), 0.0, &x1_x, &x1_y );
        d->m_matrix.map( double( d->m_mouseStart.x() ), 0.0, &x2_x, &x2_y );
        double y1_x, y1_y, y2_x, y2_y;
        d->m_matrix.map( 0.0, double( p.y() ), &y1_x, &y1_y );
        d->m_matrix.map( 0.0, double( d->m_mouseStart.y() ), &y2_x, &y2_y );

        double dx = x2_x - x1_x;
        double dy = x2_y - x1_y;
        int x = int( sqrt( dx * dx + dy * dy ) * ( d->m_mouseStart.x() < p.x() ? 1.0 : -1.0 ) );

        dx = y2_x - y1_x;
        dy = y2_y - y1_y;
        int y = int( sqrt( dx * dx + dy * dy ) * ( d->m_mouseStart.y() < p.y() ? 1.0 : -1.0 ) );

        switch( d->m_gadget )
        {
        case KoChild::TopLeft:
            {
                x = qMin( d->m_geometryStart.width() - 1, x );
                y = qMin( d->m_geometryStart.height() - 1, y );

                d->m_child->setGeometry( QRect( d->m_geometryStart.x() + x, d->m_geometryStart.y() + y,
                                             d->m_geometryStart.width() - x, d->m_geometryStart.height() - y ) );
                repaint(rgn);
            }
            break;
        case KoChild::TopMid:
            {
                y = qMin( d->m_geometryStart.height() - 1, y );

                d->m_child->setGeometry( QRect( d->m_geometryStart.x(), d->m_geometryStart.y() + y,
                                             d->m_geometryStart.width(), d->m_geometryStart.height() - y ) );
                repaint(rgn);
            }
            break;
        case KoChild::TopRight:
            {
                x = qMax( -d->m_geometryStart.width() + 1, x );
                y = qMin( d->m_geometryStart.height() - 1, y );

                d->m_child->setGeometry( QRect( d->m_geometryStart.x(), d->m_geometryStart.y() + y,
                                             d->m_geometryStart.width() + x, d->m_geometryStart.height() - y ) );
                repaint(rgn);
            }
            break;
        case KoChild::MidLeft:
            {
                x = qMin( d->m_geometryStart.width() - 1, x );

                d->m_child->setGeometry( QRect( d->m_geometryStart.x() + x, d->m_geometryStart.y(),
                                             d->m_geometryStart.width() - x, d->m_geometryStart.height() ) );
                repaint(rgn);
            }
            break;
        case KoChild::MidRight:
            {
                x = qMax( -d->m_geometryStart.width() + 1, x );

                d->m_child->setGeometry( QRect( d->m_geometryStart.x(), d->m_geometryStart.y(),
                                             d->m_geometryStart.width() + x, d->m_geometryStart.height() ) );
                repaint(rgn);
            }
            break;
        case KoChild::BottomLeft:
            {
                x = qMin( d->m_geometryStart.width() - 1, x );
                y = qMax( -d->m_geometryStart.height() + 1, y );

                d->m_child->setGeometry( QRect( d->m_geometryStart.x() + x, d->m_geometryStart.y(),
                                             d->m_geometryStart.width() - x, d->m_geometryStart.height() + y ) );
                repaint(rgn);
            }
            break;
        case KoChild::BottomMid:
            {
                y = qMax( -d->m_geometryStart.height() + 1, y );

                d->m_child->setGeometry( QRect( d->m_geometryStart.x(), d->m_geometryStart.y(),
                                             d->m_geometryStart.width(), d->m_geometryStart.height() + y ) );
                repaint(rgn);
            }
            break;
        case KoChild::BottomRight:
            {
                x = qMax( -d->m_geometryStart.width() + 1, x );
                y = qMax( -d->m_geometryStart.height() + 1, y );

                d->m_child->setGeometry( QRect( d->m_geometryStart.x(), d->m_geometryStart.y(),
                                             d->m_geometryStart.width() + x, d->m_geometryStart.height() + y ) );
                repaint(rgn);
            }
            break;
        default:
            Q_ASSERT( 0 );
        }
        return true;
    }
    return false;
}

// --------------------------------------------------------------

class KoPartMoveHandlerPrivate {
public:
    KoPartMoveHandlerPrivate( const QMatrix& matrix, KoView* view, KoChild* child,
                            const QPoint& point) : m_view(view), m_dragChild(child),
                                                   m_parentMatrix(matrix) {
        m_invertParentMatrix = matrix.inverted();
        m_mouseDragStart = m_invertParentMatrix.map( point );
        m_geometryDragStart = m_dragChild->geometry();
        m_rotationDragStart = m_dragChild->rotationPoint();
    }
    ~KoPartMoveHandlerPrivate() {}

    KoView* m_view;
    KoChild* m_dragChild;
    QPoint m_mouseDragStart;
    QRect m_geometryDragStart;
    QPoint m_rotationDragStart;
    QMatrix m_invertParentMatrix;
    QMatrix m_parentMatrix;
};

KoPartMoveHandler::KoPartMoveHandler( QWidget* widget, const QMatrix& matrix, KoView* view, KoChild* child,
                                  const QPoint& point )
    : KoEventHandler( widget )
{
    child->lock();
    d=new KoPartMoveHandlerPrivate(matrix, view, child, point);
}

KoPartMoveHandler::~KoPartMoveHandler()
{
    d->m_dragChild->unlock();
    delete d;
    d=0L;
}

bool KoPartMoveHandler::eventFilter( QObject*, QEvent* ev )
{
    if ( ev->type() == QEvent::MouseButtonRelease )
    {
        delete this;
        return true;
    }
    else if ( ev->type() == QEvent::MouseMove )
    {
        QMouseEvent* e = (QMouseEvent*)ev;

        QRegion bound = d->m_dragChild->frameRegion( d->m_parentMatrix, true );
        QPoint pos = d->m_invertParentMatrix.map( e->pos() /* + QPoint(d->m_view->canvasXOffset(), d->m_view->canvasYOffset()) */ );
        d->m_dragChild->setGeometry( QRect( d->m_geometryDragStart.x() + pos.x() - d->m_mouseDragStart.x(),
                                             d->m_geometryDragStart.y() + pos.y() - d->m_mouseDragStart.y(),
                                             d->m_geometryDragStart.width(), d->m_geometryDragStart.height() ) );
        d->m_dragChild->setRotationPoint( QPoint( d->m_rotationDragStart.x() + pos.x() - d->m_mouseDragStart.x(),
                                               d->m_rotationDragStart.y() + pos.y() - d->m_mouseDragStart.y() ) );
        bound = bound.unite( d->m_dragChild->frameRegion( d->m_parentMatrix, false ) );
      //  bound.translate(- d->m_view->canvasXOffset(), - d->m_view->canvasYOffset());
        ((QWidget*)target())->repaint( bound );

        return true;
    }

    return false;
}

// -------------------------------------------------------

KoContainerHandler::KoContainerHandler( KoView* view, QWidget* widget )
    : KoEventHandler( widget )
{
    m_view = view;
}

KoContainerHandler::~KoContainerHandler()
{
}

bool KoContainerHandler::eventFilter( QObject*, QEvent* ev )
{
    if ( ev->type() == QEvent::KeyPress )
    {
	QKeyEvent* keyEvent=(QKeyEvent*)ev;

	KoChild* child=m_view->selectedChild();

	if (child != 0)
	{
		if (keyEvent->key()==Qt::Key_Delete)
			emit deleteChild(child);
	}
    }

    if ( ev->type() == QEvent::MouseButtonPress )
    {
        KoChild::Gadget gadget;
        QPoint pos;
        QMouseEvent *e=static_cast<QMouseEvent*>(ev);
        KoChild *ch=child(gadget, pos, e);

	if ( e->button() == Qt::RightButton && gadget != KoChild::NoGadget )
        {
	    emit popupMenu( ch, e->globalPos() );
            return true;
        }
        else if ( e->button() == Qt::LeftButton && gadget == KoChild::Move )
        {
            (void)new KoPartMoveHandler( static_cast<QWidget*>(target()), m_view->matrix(), m_view, ch, pos );
            return true;
        }
        else if ( e->button() == Qt::LeftButton && gadget != KoChild::NoGadget )
        {
            (void)new KoPartResizeHandler( static_cast<QWidget*>(target()), m_view->matrix(), m_view, ch, gadget, pos );
            return true;
        }
        return false;
    }
    else if ( ev->type() == QEvent::MouseMove )
    {
        QWidget *targetWidget = static_cast<QWidget *>( target() );
        KoChild::Gadget gadget;
        QPoint pos;
        QMouseEvent *e=static_cast<QMouseEvent*>(ev);
        child(gadget, pos, e);

        bool retval = true;
        if ( gadget == KoChild::NoGadget )
            retval = false;

        if ( gadget == KoChild::TopLeft || gadget == KoChild::BottomRight )
            targetWidget->setCursor( Qt::SizeFDiagCursor );
        else if ( gadget == KoChild::TopRight || gadget == KoChild::BottomLeft )
            targetWidget->setCursor( Qt::SizeBDiagCursor );
        else if ( gadget == KoChild::TopMid || gadget == KoChild::BottomMid )
            targetWidget->setCursor( Qt::SizeHorCursor );
        else if ( gadget == KoChild::MidLeft || gadget == KoChild::MidRight )
            targetWidget->setCursor( Qt::SizeHorCursor );
        else if ( gadget == KoChild::Move )
            targetWidget->setCursor( KCursor::handCursor() );
        else
        {
//            targetWidget->setCursor( arrowCursor );
            return false;
        }
        return retval;
    }
    return false;
}

KoChild *KoContainerHandler::child(KoChild::Gadget &gadget, QPoint &pos, const QMouseEvent *ev) {

    pos = ev->pos(); //+ QPoint(m_view->canvasXOffset(), m_view->canvasYOffset());

    pos = m_view->reverseViewTransformations( pos );

    KoChild *child = 0;
    KoDocumentChild* docChild = m_view->selectedChild();
    gadget = KoChild::NoGadget;
    if ( docChild )
    {
        KoViewChild *viewChild = m_view->child( docChild->document() );

        if ( viewChild )
            child = viewChild;
        else
            child = docChild;

        gadget = child->gadgetHitTest( pos );
    }
    if ( gadget == KoChild::NoGadget )
    {
        docChild = m_view->activeChild();
        if ( docChild )
        {
            KoViewChild *viewChild = m_view->child( docChild->document() );

            if ( viewChild )
                child = viewChild;
            else
                child = docChild;

            gadget = child->gadgetHitTest( pos );
        }
    }
    return child;
}

#include "KoContainerHandler.moc"
