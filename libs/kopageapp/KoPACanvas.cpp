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

class KoPACanvas::Private
{
public:
    Private(KoPAView * view, KoPADocument * doc)
    : view(view)
    , doc(doc)
    , shapeManager(0)
    , masterShapeManager(0)
    , toolProxy(0)
    {}

    ~Private()
    {
        delete toolProxy;
        delete masterShapeManager;
        delete shapeManager;
    }

    ///< the origin of the page rect inside the canvas in document points
    QPointF origin() const
    {
        return view->viewMode()->origin();
    }

    KoPAView * view;
    KoPADocument * doc;
    KoShapeManager * shapeManager;
    KoShapeManager * masterShapeManager;
    KoToolProxy * toolProxy;
    QPoint documentOffset;
};

KoPACanvas::KoPACanvas( KoPAView * view, KoPADocument * doc )
: QWidget( view )
, KoCanvasBase( doc )
, d(new Private(view, doc))
{
    d->shapeManager = new KoShapeManager( this );
    d->masterShapeManager = new KoShapeManager( this );
    d->toolProxy = new KoToolProxy( this );
    setFocusPolicy( Qt::StrongFocus );
    // this is much faster than painting it in the paintevent
    setBackgroundRole( QPalette::Base );
    setAutoFillBackground( true );
    updateSize();
    setAttribute(Qt::WA_InputMethodEnabled, true);
}

KoPACanvas::~KoPACanvas()
{
    delete d;
}

KoPADocument* KoPACanvas::document() const
{
    return d->doc;
}

KoToolProxy* KoPACanvas::toolProxy() const
{
    return d->toolProxy;
}

QWidget* KoPACanvas::canvasWidget()
{
    return this;
}

const QWidget* KoPACanvas::canvasWidget() const
{
    return this;
}

KoPAView* KoPACanvas::koPAView() const
{
    return d->view;
}

void KoPACanvas::updateSize()
{
    QSize size;

    if ( d->view->activePage() ) {
        KoPageLayout pageLayout = d->view->activePage()->pageLayout();
        size.setWidth( qRound( d->view->zoomHandler()->zoomItX( pageLayout.width ) ) );
        size.setHeight( qRound( d->view->zoomHandler()->zoomItX( pageLayout.height ) ) );
    }

    emit documentSize(size);
}

void KoPACanvas::setDocumentOffset(const QPoint &offset) {
    d->documentOffset = offset;
}

QPoint KoPACanvas::documentOrigin() const
{
    return viewConverter()->documentToView(d->origin()).toPoint();
}

void KoPACanvas::setDocumentOrigin(const QPointF & o)
{
    d->view->viewMode()->setOrigin(o);
}

void KoPACanvas::gridSize( qreal *horizontal, qreal *vertical ) const
{
    *horizontal = d->doc->gridData().gridX();
    *vertical = d->doc->gridData().gridY();
}

bool KoPACanvas::snapToGrid() const
{
    return d->doc->gridData().snapToGrid();
}

void KoPACanvas::addCommand( QUndoCommand *command )
{
    d->doc->addCommand( command );
}

KoShapeManager * KoPACanvas::shapeManager() const
{
    return d->shapeManager;
}

KoShapeManager * KoPACanvas::masterShapeManager() const
{
    return d->masterShapeManager;
}

void KoPACanvas::updateCanvas( const QRectF& rc )
{
    QRect clipRect(viewToWidget(viewConverter()->documentToView(rc).toRect()));
    clipRect.adjust( -2, -2, 2, 2 ); // Resize to fit anti-aliasing
    clipRect.moveTopLeft( clipRect.topLeft() - d->documentOffset);
    update( clipRect );

    emit canvasUpdated();
}

const KoViewConverter * KoPACanvas::viewConverter() const
{
    return d->view->viewMode()->viewConverter( const_cast<KoPACanvas *>( this ) );
}

KoUnit KoPACanvas::unit() const
{
    return d->doc->unit();
}

const QPoint & KoPACanvas::documentOffset() const
{
    return d->documentOffset;
}

void KoPACanvas::paintEvent( QPaintEvent *event )
{
    KoPAPageBase *activePage(d->view->activePage());
    if (d->view->activePage()) {
        int pageNumber = d->doc->pageIndex( d->view->activePage() ) + 1;
        QVariant var = d->doc->resourceManager()->resource(KoText::PageProvider);
        static_cast<KoPAPageProvider*>(var.value<void*>())->setPageData(pageNumber, activePage);
        d->view->viewMode()->paintEvent( this, event );
    }
}

void KoPACanvas::tabletEvent( QTabletEvent *event )
{
    d->view->viewMode()->tabletEvent(event, viewConverter()->viewToDocument(widgetToView(event->pos() + d->documentOffset)));
}

void KoPACanvas::mousePressEvent( QMouseEvent *event )
{
    d->view->viewMode()->mousePressEvent(event, viewConverter()->viewToDocument(widgetToView(event->pos() + d->documentOffset)));

    if(!event->isAccepted() && event->button() == Qt::RightButton)
    {
        showContextMenu( event->globalPos(), toolProxy()->popupActionList() );
        event->setAccepted( true );
    }
}

void KoPACanvas::mouseDoubleClickEvent( QMouseEvent *event )
{
    d->view->viewMode()->mouseDoubleClickEvent( event, viewConverter()->viewToDocument(widgetToView(event->pos() + d->documentOffset)));
}

void KoPACanvas::mouseMoveEvent( QMouseEvent *event )
{
    d->view->viewMode()->mouseMoveEvent( event, viewConverter()->viewToDocument(widgetToView(event->pos() + d->documentOffset)));
}

void KoPACanvas::mouseReleaseEvent( QMouseEvent *event )
{
    d->view->viewMode()->mouseReleaseEvent( event, viewConverter()->viewToDocument(widgetToView(event->pos() + d->documentOffset)));
}

void KoPACanvas::keyPressEvent( QKeyEvent *event )
{
    d->view->viewMode()->keyPressEvent( event );
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
    d->view->viewMode()->keyReleaseEvent( event );
}

void KoPACanvas::wheelEvent ( QWheelEvent * event )
{
    d->view->viewMode()->wheelEvent( event, viewConverter()->viewToDocument(widgetToView(event->pos() + d->documentOffset)));
}

void KoPACanvas::closeEvent( QCloseEvent * event )
{
    d->view->viewMode()->closeEvent( event );
}

void KoPACanvas::updateInputMethodInfo()
{
    updateMicroFocus();
}

QVariant KoPACanvas::inputMethodQuery(Qt::InputMethodQuery query) const
{
    return d->toolProxy->inputMethodQuery(query, *(viewConverter()) );
}

void KoPACanvas::inputMethodEvent(QInputMethodEvent *event)
{
    d->toolProxy->inputMethodEvent(event);
}

void KoPACanvas::resizeEvent( QResizeEvent * event )
{
    emit sizeChanged( event->size() );
}

void KoPACanvas::showContextMenu( const QPoint& globalPos, const QList<QAction*>& actionList )
{
    d->view->unplugActionList( "toolproxy_action_list" );
    d->view->plugActionList( "toolproxy_action_list", actionList );
    if( !d->view->factory() ) return;

    QMenu *menu = dynamic_cast<QMenu*>( d->view->factory()->container( "default_canvas_popup", d->view ) );

    if( menu )
        menu->exec( globalPos );
}

QPoint KoPACanvas::widgetToView(const QPoint& p) const
{
    return p - viewConverter()->documentToView(d->origin()).toPoint();
}

QRect KoPACanvas::widgetToView(const QRect& r) const
{
    return r.translated(viewConverter()->documentToView(-d->origin()).toPoint());
}

QPoint KoPACanvas::viewToWidget(const QPoint& p) const
{
    return p + viewConverter()->documentToView(d->origin()).toPoint();
}

QRect KoPACanvas::viewToWidget(const QRect& r) const
{
    return r.translated(viewConverter()->documentToView(d->origin()).toPoint());
}

KoGuidesData * KoPACanvas::guidesData()
{
    return &d->doc->guidesData();
}

void KoPACanvas::setCursor(const QCursor &cursor)
{
    QWidget::setCursor(cursor);
}

#include <KoPACanvas.moc>
