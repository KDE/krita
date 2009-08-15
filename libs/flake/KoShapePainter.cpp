/* This file is part of the KDE project
 *
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
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

#include "KoShapePainter.h"

#include <KoCanvasBase.h>
#include <KoShapeManager.h>
#include <KoShapeManagerPaintingStrategy.h>
#include <KoZoomHandler.h>
#include <KoUnit.h>
#include <KoShape.h>
#include <KoShapeBorderModel.h>
#include <KoShapeGroup.h>
#include <KoShapeContainer.h>

#include <QtGui/QImage>

class SimpleCanvas : public KoCanvasBase
{
public:
    SimpleCanvas()
        : KoCanvasBase(0), m_shapeManager( new KoShapeManager( this ) )
        , m_zoomHandler( new KoZoomHandler() )
    {
    }

    ~SimpleCanvas()
    {
        delete m_shapeManager;
        delete m_zoomHandler;
    }

    virtual void gridSize(qreal *horizontal, qreal *vertical) const
    {
        if( horizontal )
            *horizontal = 0;
        if( vertical )
            *vertical = 0;
    };

    virtual bool snapToGrid() const
    {
        return false;
    }

    virtual void addCommand(QUndoCommand *command)
    {
        Q_UNUSED( command );
    };

    virtual KoShapeManager *shapeManager() const
    {
        return m_shapeManager;
    };

    virtual void updateCanvas(const QRectF& rc)
    {
        Q_UNUSED( rc );
    };

    virtual KoToolProxy * toolProxy() const
    {
        return 0;
    };

    virtual const KoViewConverter *viewConverter() const
    {
        return m_zoomHandler;
    }

    virtual QWidget* canvasWidget()
    {
        return 0;
    };

    virtual const QWidget* canvasWidget() const {
        return 0;
    }

    virtual KoUnit unit() const
    {
        return KoUnit( KoUnit::Point );
    }

    virtual void updateInputMethodInfo() {};
private:
    KoShapeManager * m_shapeManager;
    KoZoomHandler * m_zoomHandler;
};

class KoShapePainter::Private
{
public:
    Private()
    : canvas( new SimpleCanvas() )
    {
    }
    ~Private() { delete canvas; }
    SimpleCanvas * canvas;
};

KoShapePainter::KoShapePainter(KoShapeManagerPaintingStrategy * strategy)
    : d( new Private() )
{
    if (strategy) {
        strategy->setShapeManager(d->canvas->shapeManager());
        d->canvas->shapeManager()->setPaintingStrategy(strategy);
    }
}

KoShapePainter::~KoShapePainter()
{
    delete d;
}

void KoShapePainter::setShapes( const QList<KoShape*> &shapes )
{
    d->canvas->shapeManager()->setShapes(shapes, KoShapeManager::AddWithoutRepaint);
}

void KoShapePainter::paintShapes( QPainter & painter, KoViewConverter & converter )
{
    foreach (KoShape *shape, d->canvas->shapeManager()->shapes()) {
        shape->waitUntilReady(converter, false);
    }

    d->canvas->shapeManager()->paint( painter, converter, true );
}

void KoShapePainter::paintShapes( QPainter & painter, const QRect & painterRect, const QRectF & documentRect )
{
    KoZoomHandler zoomHandler;
    // calculate the painter destination rectangle size in document coordinates
    QRectF paintBox = zoomHandler.viewToDocument(QRectF(QPointF(), painterRect.size()));

    // compute the zoom factor based on the bounding rects in document coordinates
    // so that the content fits into the image
    qreal zoomW = paintBox.width() / documentRect.width();
    qreal zoomH = paintBox.height() / documentRect.height();
    qreal zoom = qMin( zoomW, zoomH );

    // now set the zoom into the zoom handler used for painting the shape
    zoomHandler.setZoom( zoom );

    painter.save();

    // initialize painter
    painter.setPen( QPen(Qt::NoPen) );
    painter.setBrush( Qt::NoBrush );
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setClipRect( painterRect.adjusted(-1,-1,1,1) );

    // convert document rectangle to view coordinates
    QRectF zoomedBound = zoomHandler.documentToView( documentRect );
    // calculate offset between painter rectangle and converted document rectangle
    QPointF offset = QPointF(painterRect.center()) - zoomedBound.center();
    // center content in painter rectangle
    painter.translate( offset.x(), offset.y() );

    // finally paint the shapes
    paintShapes( painter, zoomHandler );

    painter.restore();
}

bool KoShapePainter::paintShapes( QImage & image )
{
    if( image.isNull() )
        return false;

    QPainter painter( &image );

    paintShapes( painter, image.rect(), contentRect() );

    return true;
}

QRectF KoShapePainter::contentRect()
{
    QRectF bound;
    foreach(KoShape * shape, d->canvas->shapeManager()->shapes()) {
        if (! shape->isVisible( true ))
            continue;
        if (dynamic_cast<KoShapeGroup*>(shape))
            continue;

        QRectF shapeRect = shape->boundingRect();

        if (bound.isEmpty())
            bound = shapeRect;
        else
            bound = bound.united(shapeRect);
    }
    return bound;
}
