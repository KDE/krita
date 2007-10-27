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
#include <KoZoomHandler.h>
#include <KoUnit.h>
#include <KoShape.h>
#include <KoShapeBorderModel.h>

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

    virtual void gridSize(double *horizontal, double *vertical) const
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

    virtual void addCommand(QUndoCommand *command) {};

    virtual KoShapeManager *shapeManager() const
    {
        return m_shapeManager;
    };

    virtual void updateCanvas(const QRectF& rc) {};

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

KoShapePainter::KoShapePainter()
    : d( new Private() )
{
}

KoShapePainter::~KoShapePainter()
{
    delete d;
}

void KoShapePainter::setShapes( const QList<KoShape*> &shapes )
{
    d->canvas->shapeManager()->setShapes( shapes, false );
}

void KoShapePainter::paintShapes( QPainter & painter, KoViewConverter & converter )
{
    d->canvas->shapeManager()->paint( painter, converter, true );
}

bool KoShapePainter::paintShapes( QImage & image )
{
    if( image.isNull() )
        return false;

    QRectF bound = contentRect();
    QSizeF size = image.size();

    KoZoomHandler zoomHandler;
    // calculate the image size in document coordinates
    QRectF imageBox = zoomHandler.viewToDocument( QRectF( 0, 0, size.width(), size.height() ) );

    // compute the zoom factor based on the bounding rects in document coordinates
    // so that the content fits into the image
    double zoomW = imageBox.width() / bound.width();
    double zoomH = imageBox.height() / bound.height();
    double zoom = qMin( zoomW, zoomH );

    // now set the zoom into the zoom handler used for painting the shape
    zoomHandler.setZoom( zoom );

    QPainter painter( &image );

    // initialize painter
    painter.setPen( QPen(Qt::NoPen) );
    painter.setBrush( Qt::NoBrush );
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setClipRect( QRectF(QPoint(),size) );

    QRectF zoomedBound = zoomHandler.documentToView( bound );
    QPointF offset = QPointF( 0.5 * size.width(), 0.5 * size.height() ) - zoomedBound.center();

    // center content in image
    painter.translate( offset.x(), offset.y() );

    // finally paint the shapes
    paintShapes( painter, zoomHandler );

    return true;
}

QRectF KoShapePainter::contentRect()
{
    QRectF bound;
    foreach( KoShape * shape, d->canvas->shapeManager()->shapes() )
    {
        QPainterPath outline = shape->absoluteTransformation(0).map( shape->outline() );
        QRectF shapeRect = outline.boundingRect();
        // correct shape box with border sizes
        if( shape->border() )
        {
            KoInsets inset;
            shape->border()->borderInsets( shape, inset );
            shapeRect.adjust( -inset.left, -inset.top, inset.right, inset.bottom );
        }
        if( bound.isEmpty() )
            bound = shapeRect;
        else
            bound = bound.united( shapeRect );
    }
    return bound;
}
