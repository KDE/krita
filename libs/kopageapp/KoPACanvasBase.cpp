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

#include "KoPACanvasBase.h"

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

class KoPACanvasBase::Private
{
public:
    Private(KoPAViewBase * view, KoPADocument * doc)
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

    KoPAViewBase * view;
    KoPADocument * doc;
    KoShapeManager * shapeManager;
    KoShapeManager * masterShapeManager;
    KoToolProxy * toolProxy;
    QPoint documentOffset;
};

KoPACanvasBase::KoPACanvasBase( KoPAViewBase * view, KoPADocument * doc )
    : KoCanvasBase( doc )
    , d(new Private(view, doc))
{
    d->shapeManager = new KoShapeManager( this );
    d->masterShapeManager = new KoShapeManager( this );
    d->toolProxy = new KoToolProxy( this );
}

KoPACanvasBase::~KoPACanvasBase()
{
    delete d;
}

KoPADocument* KoPACanvasBase::document() const
{
    return d->doc;
}

KoToolProxy* KoPACanvasBase::toolProxy() const
{
    return d->toolProxy;
}

KoPAViewBase* KoPACanvasBase::koPAView() const
{
    return d->view;
}

QPoint KoPACanvasBase::documentOrigin() const
{
    return viewConverter()->documentToView(d->origin()).toPoint();
}

void KoPACanvasBase::setDocumentOrigin(const QPointF & o)
{
    d->view->viewMode()->setOrigin(o);
}

void KoPACanvasBase::gridSize( qreal *horizontal, qreal *vertical ) const
{
    *horizontal = d->doc->gridData().gridX();
    *vertical = d->doc->gridData().gridY();
}

bool KoPACanvasBase::snapToGrid() const
{
    return d->doc->gridData().snapToGrid();
}

void KoPACanvasBase::addCommand( QUndoCommand *command )
{
    d->doc->addCommand( command );
}

KoShapeManager * KoPACanvasBase::shapeManager() const
{
    return d->shapeManager;
}

KoShapeManager * KoPACanvasBase::masterShapeManager() const
{
    return d->masterShapeManager;
}

const KoViewConverter * KoPACanvasBase::viewConverter() const
{
    return d->view->viewMode()->viewConverter( const_cast<KoPACanvasBase *>( this ) );
}

KoUnit KoPACanvasBase::unit() const
{
    return d->doc->unit();
}

const QPoint & KoPACanvasBase::documentOffset() const
{
    return d->documentOffset;
}

void KoPACanvasBase::setDocumentOffset(const QPoint &offset) {
    d->documentOffset = offset;
}

QPoint KoPACanvasBase::widgetToView(const QPoint& p) const
{
    return p - viewConverter()->documentToView(d->origin()).toPoint();
}

QRect KoPACanvasBase::widgetToView(const QRect& r) const
{
    return r.translated(viewConverter()->documentToView(-d->origin()).toPoint());
}

QPoint KoPACanvasBase::viewToWidget(const QPoint& p) const
{
    return p + viewConverter()->documentToView(d->origin()).toPoint();
}

QRect KoPACanvasBase::viewToWidget(const QRect& r) const
{
    return r.translated(viewConverter()->documentToView(d->origin()).toPoint());
}

KoGuidesData * KoPACanvasBase::guidesData()
{
    return &d->doc->guidesData();
}

void KoPACanvasBase::paint(QPainter &painter, const QRectF paintRect) {

    KoPAPageBase *activePage(d->view->activePage());
    if (d->view->activePage()) {
        int pageNumber = d->doc->pageIndex( d->view->activePage() ) + 1;
        QVariant var = d->doc->resourceManager()->resource(KoText::PageProvider);
        static_cast<KoPAPageProvider*>(var.value<void*>())->setPageData(pageNumber, activePage);
        d->view->viewMode()->paint(this, painter, paintRect);
    }
}
