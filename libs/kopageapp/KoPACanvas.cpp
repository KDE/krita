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
#include <KoText.h>

#include "KoPADocument.h"
#include "KoPAView.h"
#include "KoPAViewMode.h"
#include "KoPAPage.h"
#include "KoPAPageProvider.h"

#include <kxmlguifactory.h>

#include <KAction>
#include <QMenu>
#include <QMouseEvent>

KoPACanvas::KoPACanvas( KoPAViewBase * view, KoPADocument * doc, QWidget *parent ,  Qt::WindowFlags f)
    : QWidget( parent, f )
    , KoPACanvasBase( doc )
{
    setView(view);
    setFocusPolicy( Qt::StrongFocus );
    // this is much faster than painting it in the paintevent
    setBackgroundRole( QPalette::Base );
    setAutoFillBackground( true );
    updateSize();
    setAttribute(Qt::WA_InputMethodEnabled, true);
}

void KoPACanvas::repaint()
{
    update();
}


QWidget* KoPACanvas::canvasWidget()
{
    return this;
}

const QWidget* KoPACanvas::canvasWidget() const
{
    return this;
}

void KoPACanvas::updateSize()
{
    QSize size;

    if ( koPAView()->activePage() ) {
        KoPageLayout pageLayout = koPAView()->activePage()->pageLayout();
        size.setWidth( qRound( koPAView()->zoomHandler()->zoomItX( pageLayout.width ) ) );
        size.setHeight( qRound( koPAView()->zoomHandler()->zoomItX( pageLayout.height ) ) );
    }

    emit documentSize(size);
}

void KoPACanvas::updateCanvas( const QRectF& rc )
{
    QRect clipRect(viewToWidget(viewConverter()->documentToView(rc).toRect()));
    clipRect.adjust( -2, -2, 2, 2 ); // Resize to fit anti-aliasing
    clipRect.moveTopLeft( clipRect.topLeft() - documentOffset());
    update( clipRect );

    emit canvasUpdated();
}

void KoPACanvas::paintEvent( QPaintEvent *event )
{
    QPainter painter(this);
    paint(painter, event->rect());
    painter.end();
}

void KoPACanvas::tabletEvent( QTabletEvent *event )
{
    koPAView()->viewMode()->tabletEvent(event, viewConverter()->viewToDocument(widgetToView(event->pos() + documentOffset())));
}

void KoPACanvas::mousePressEvent( QMouseEvent *event )
{
    koPAView()->viewMode()->mousePressEvent(event, viewConverter()->viewToDocument(widgetToView(event->pos() + documentOffset())));

    if(!event->isAccepted() && event->button() == Qt::RightButton)
    {
        showContextMenu( event->globalPos(), toolProxy()->popupActionList() );
        event->setAccepted( true );
    }
}

void KoPACanvas::mouseDoubleClickEvent( QMouseEvent *event )
{
    koPAView()->viewMode()->mouseDoubleClickEvent( event, viewConverter()->viewToDocument(widgetToView(event->pos() + documentOffset())));
}

void KoPACanvas::mouseMoveEvent( QMouseEvent *event )
{
    koPAView()->viewMode()->mouseMoveEvent( event, viewConverter()->viewToDocument(widgetToView(event->pos() + documentOffset())));
}

void KoPACanvas::mouseReleaseEvent( QMouseEvent *event )
{
    koPAView()->viewMode()->mouseReleaseEvent( event, viewConverter()->viewToDocument(widgetToView(event->pos() + documentOffset())));
}

void KoPACanvas::keyPressEvent( QKeyEvent *event )
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

void KoPACanvas::keyReleaseEvent( QKeyEvent *event )
{
    koPAView()->viewMode()->keyReleaseEvent( event );
}

void KoPACanvas::wheelEvent ( QWheelEvent * event )
{
    koPAView()->viewMode()->wheelEvent( event, viewConverter()->viewToDocument(widgetToView(event->pos() + documentOffset())));
}

void KoPACanvas::closeEvent( QCloseEvent * event )
{
    koPAView()->viewMode()->closeEvent( event );
}

void KoPACanvas::updateInputMethodInfo()
{
    updateMicroFocus();
}

QVariant KoPACanvas::inputMethodQuery(Qt::InputMethodQuery query) const
{
    return toolProxy()->inputMethodQuery(query, *(viewConverter()) );
}

void KoPACanvas::inputMethodEvent(QInputMethodEvent *event)
{
    toolProxy()->inputMethodEvent(event);
}

void KoPACanvas::resizeEvent( QResizeEvent * event )
{
    emit sizeChanged( event->size() );
}

void KoPACanvas::showContextMenu( const QPoint& globalPos, const QList<QAction*>& actionList )
{
    KoPAView *view = dynamic_cast<KoPAView*>(koPAView());
    if (!view || !view->factory()) return;

    view->unplugActionList( "toolproxy_action_list" );
    view->plugActionList( "toolproxy_action_list", actionList );


    QMenu *menu = dynamic_cast<QMenu*>( view->factory()->container( "default_canvas_popup", view ) );

    if( menu )
        menu->exec( globalPos );
}

void KoPACanvas::setCursor(const QCursor &cursor) 
{
    QWidget::setCursor(cursor);
}

#include <KoPACanvas.moc>
