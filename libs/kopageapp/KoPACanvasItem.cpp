/* This file is part of the KDE project
   Copyright (C) 2006-2007 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2010 Boudewijn Rempt <boud@valdyas.org>

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
   Boston, MA 02110-1301, USA.
*/

#include "KoPACanvasItem.h"

#include <QStyleOptionGraphicsItem>
#include <QGraphicsSceneResizeEvent>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>

#include <KoShapeManager.h>
#include <KoToolProxy.h>
#include <KoUnit.h>
#include <KoText.h>

#include "KoPADocument.h"
#include "KoPAView.h"
#include "KoPAViewMode.h"
#include "KoPAPage.h"
#include "KoPAPageProvider.h"

#include <kxmlguifactory.h>

#include <KAction>
#include <QMenu>
#include <QGraphicsSceneMouseEvent>

KoPACanvasItem::KoPACanvasItem( KoPADocument * doc)
    : QGraphicsWidget()
    , KoPACanvasBase( doc )
{
    setFocusPolicy( Qt::StrongFocus );
#if QT_VERSION  >= 0x040700
    // this is much faster than painting it in the paintevent
    setAutoFillBackground( true );
#endif
}

void KoPACanvasItem::repaint()
{
    update();
}


void KoPACanvasItem::updateSize()
{
    QSize size;

    if ( koPAView()->activePage() ) {
        KoPageLayout pageLayout = koPAView()->activePage()->pageLayout();
        size.setWidth( qRound( koPAView()->zoomHandler()->zoomItX( pageLayout.width ) ) );
        size.setHeight( qRound( koPAView()->zoomHandler()->zoomItX( pageLayout.height ) ) );
    }
    emit documentSize(size);
}

void KoPACanvasItem::updateCanvas( const QRectF& rc )
{    QRect clipRect(viewToWidget(viewConverter()->documentToView(rc).toRect()));
    clipRect.adjust( -2, -2, 2, 2 ); // Resize to fit anti-aliasing
    clipRect.moveTopLeft( clipRect.topLeft() - documentOffset());
    update( clipRect );

    emit canvasUpdated();
}

void KoPACanvasItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget */*widget*/)
{
    KoPACanvasBase::paint(*painter, option->exposedRect);
}


void KoPACanvasItem::mousePressEvent( QGraphicsSceneMouseEvent *e )
{
    QMouseEvent me(e->type(), e->pos().toPoint(), e->button(), e->buttons(), e->modifiers());
    Q_ASSERT(koPAView());
    Q_ASSERT(koPAView()->viewMode());
    Q_ASSERT(viewConverter());
    koPAView()->viewMode()->mousePressEvent(&me, viewConverter()->viewToDocument(widgetToView(me.pos() + documentOffset())));

    if(!me.isAccepted() && me.button() == Qt::RightButton)
    {
        showContextMenu( me.pos(), toolProxy()->popupActionList() );
        e->setAccepted( true );
    }
}

void KoPACanvasItem::mouseDoubleClickEvent( QGraphicsSceneMouseEvent *e )
{
    QMouseEvent me(e->type(), e->pos().toPoint(), e->button(), e->buttons(), e->modifiers());
    koPAView()->viewMode()->mouseDoubleClickEvent(&me, viewConverter()->viewToDocument(widgetToView(me.pos() + documentOffset())));
}

void KoPACanvasItem::mouseMoveEvent( QGraphicsSceneMouseEvent *e )
{
    QMouseEvent me(e->type(), e->pos().toPoint(), e->button(), e->buttons(), e->modifiers());
    koPAView()->viewMode()->mouseMoveEvent(&me, viewConverter()->viewToDocument(widgetToView(me.pos() + documentOffset())));
}

void KoPACanvasItem::mouseReleaseEvent( QGraphicsSceneMouseEvent *e )
{
    QMouseEvent me(e->type(), e->pos().toPoint(), e->button(), e->buttons(), e->modifiers());
    koPAView()->viewMode()->mouseReleaseEvent(&me, viewConverter()->viewToDocument(widgetToView(me.pos() + documentOffset())));
}

void KoPACanvasItem::keyPressEvent( QKeyEvent *event )
{
    koPAView()->viewMode()->keyPressEvent( event );
    if (! event->isAccepted()) {
        if (event->key() == Qt::Key_Backtab
                || (event->key() == Qt::Key_Tab && (event->modifiers() & Qt::ShiftModifier)))
            focusNextPrevChild(false);
        else if (event->key() == Qt::Key_Tab)
            focusNextPrevChild(true);
    }
}

void KoPACanvasItem::keyReleaseEvent( QKeyEvent *event )
{
    koPAView()->viewMode()->keyReleaseEvent( event );
}

void KoPACanvasItem::wheelEvent ( QGraphicsSceneWheelEvent * event )
{
    QWheelEvent ev(event->pos().toPoint(), event->delta(), event->buttons(), event->modifiers(), event->orientation());
    koPAView()->viewMode()->wheelEvent( &ev, viewConverter()->viewToDocument(widgetToView(ev.pos() + documentOffset())));
}

void KoPACanvasItem::closeEvent( QCloseEvent * event )
{
    koPAView()->viewMode()->closeEvent( event );
}

void KoPACanvasItem::updateInputMethodInfo()
{
#if QT_VERSION  >= 0x040700
    updateMicroFocus();
#endif
}

QVariant KoPACanvasItem::inputMethodQuery(Qt::InputMethodQuery query) const
{
    return toolProxy()->inputMethodQuery(query, *(viewConverter()) );
}

void KoPACanvasItem::inputMethodEvent(QInputMethodEvent *event)
{
    toolProxy()->inputMethodEvent(event);
}

void KoPACanvasItem::resizeEvent( QGraphicsSceneResizeEvent * event )
{
    emit sizeChanged( event->newSize().toSize() );
}

void KoPACanvasItem::showContextMenu( const QPoint& globalPos, const QList<QAction*>& actionList )
{
    KoPAView* view = dynamic_cast<KoPAView*>(koPAView());
    if (!view) return;

    view->unplugActionList( "toolproxy_action_list" );
    view->plugActionList( "toolproxy_action_list", actionList );
    if( !view->factory() ) return;

    QMenu *menu = dynamic_cast<QMenu*>( view->factory()->container( "default_canvas_popup", view ) );

    if( menu )
        menu->exec( globalPos );
}

void KoPACanvasItem::setCursor(const QCursor &cursor)
{
    QGraphicsWidget::setCursor(cursor);
}

#include <KoPACanvasItem.moc>
