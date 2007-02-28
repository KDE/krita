/* This file is part of the KDE project

   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2006 Thomas Zander <zander@kde.org>

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

#include "KoShapeManager.h"
#include "KoSelection.h"
#include "KoShape.h"
#include "KoShapeConnection.h"
#include "KoCanvasBase.h"
#include "KoShapeContainer.h"
#include "KoShapeBorderModel.h"
#include "KoShapeGroup.h"
#include "KoToolProxy.h"

#include <QPainter>
#include <kdebug.h>

class KoShapeManager::Private {
public:
    Private(KoCanvasBase *c) : canvas(c), tree(4, 2), connectionTree(4, 2) {
        selection = new KoSelection();
    }
    QList<KoShape *> shapes;
    KoSelection * selection;
    KoCanvasBase * canvas;
    KoRTree<KoShape *> tree;
    KoRTree<KoShapeConnection *> connectionTree;
    QSet<KoShape *> aggregate4update;
    QHash<KoShape*, int> shapeIndexesBeforeUpdate;
};

KoShapeManager::KoShapeManager( KoCanvasBase *canvas, const QList<KoShape *> &shapes )
: d (new Private(canvas))
{
    Q_ASSERT(d->canvas); // not optional.
    connect( d->selection, SIGNAL(selectionChanged()), this, SIGNAL(selectionChanged()) );
    setShapes(shapes);
}

KoShapeManager::KoShapeManager(KoCanvasBase *canvas)
: d (new Private(canvas))
{
    Q_ASSERT(d->canvas); // not optional.
    connect( d->selection, SIGNAL(selectionChanged()), this, SIGNAL(selectionChanged()) );
}

KoShapeManager::~KoShapeManager()
{
    foreach(KoShape *shape, d->shapes)
        shape->removeShapeManager( this );
    delete d->selection;
}


void KoShapeManager::setShapes( const QList<KoShape *> &shapes )
{
    //clear selection
    d->selection->deselectAll();
    foreach(KoShape *shape, d->shapes)
    {
        d->aggregate4update.remove( shape );
        d->tree.remove( shape );
        shape->removeShapeManager( this );
    }
    d->shapes.clear();
    foreach(KoShape *shape, shapes)
    {
        add( shape );
    }
}

void KoShapeManager::add( KoShape *shape )
{
    if(d->shapes.contains(shape))
        return;
    shape->addShapeManager( this );
    d->shapes.append(shape);
    if( ! dynamic_cast<KoShapeGroup*>( shape ))
    {
        QRectF br( shape->boundingRect() );
        d->tree.insert( br, shape );
    }
    shape->repaint();

    // add the children of a KoShapeContainer
    KoShapeContainer* container = dynamic_cast<KoShapeContainer*>(shape);

    if(container)
    {
        foreach(KoShape* containerShape, container->iterator())
        {
            add(containerShape);
        }
    }
}

void KoShapeManager::remove( KoShape *shape )
{
    shape->repaint();
    shape->removeShapeManager( this );
    d->selection->deselect( shape );
    d->aggregate4update.remove( shape );
    d->tree.remove( shape );
    d->shapes.removeAll(shape);

    // remove the children of a KoShapeContainer
    KoShapeContainer* container = dynamic_cast<KoShapeContainer*>(shape);

    if(container)
    {
        foreach(KoShape* containerShape, container->iterator())
        {
            remove(containerShape);
        }
    }
}

void KoShapeManager::paint( QPainter &painter, const KoViewConverter &converter, bool forPrint)
{
    updateTree();
    painter.setPen( Qt::NoPen );// painters by default have a black stroke, lets turn that off.
    painter.setBrush( Qt::NoBrush );

    QList<KoShapeConnection*> sortedConnections;
    QList<KoShape*> sortedShapes;
    if(painter.hasClipping()) {
        QRectF rect = converter.viewToDocument( painter.clipRegion().boundingRect() );
        sortedShapes = d->tree.intersects( rect );
        sortedConnections = d->connectionTree.intersects( rect );
    }
    else {
        sortedShapes = shapes();
        kWarning() << "KoShapeManager::paint  Painting with a painter that has no clipping will lead to too much being painted and no connections being painted!\n";
    }
    qSort(sortedShapes.begin(), sortedShapes.end(), KoShape::compareShapeZIndex);
    qSort(sortedConnections.begin(), sortedConnections.end(), KoShapeConnection::compareConnectionZIndex);
    QList<KoShapeConnection*>::iterator connectionIterator = sortedConnections.begin();

    const QRegion clipRegion = painter.clipRegion();
    foreach ( KoShape * shape, sortedShapes ) {
        if(! shape->isVisible() || ( shape->parent() && ! shape->parent()->isVisible() ) )
            continue;
        if(shape->parent() != 0 && shape->parent()->childClipped(shape))
            continue;
        if(painter.hasClipping()) {
            QRectF shapeBox = shape->boundingRect();
            shapeBox = converter.documentToView(shapeBox);
            QRegion shapeRegion = QRegion(shapeBox.toRect());

            if(clipRegion.intersect(shapeRegion).isEmpty())
                continue;
        }

        while(connectionIterator != sortedConnections.end() && (*connectionIterator)->zIndex() < shape->zIndex()) {
            painter.save();
            (*connectionIterator)->paint( painter, converter );
            painter.restore();
            connectionIterator++;
        }
        painter.save();
        painter.setMatrix( shape->transformationMatrix(&converter) * painter.matrix() );

        painter.save();
        shape->paint( painter, converter );
        painter.restore();
        if(shape->border()) {
            painter.save();
            shape->border()->paintBorder(shape, painter, converter);
            painter.restore();
        }
        if(! forPrint) {
            painter.save();
            painter.setRenderHint( QPainter::Antialiasing, false );
            shape->paintDecorations( painter, converter, d->canvas );
            painter.restore();
        }
        painter.restore();  // for the matrix
    }

    while(connectionIterator != sortedConnections.end()) { // paint connections that are above the rest.
        painter.save();
        (*connectionIterator)->paint( painter, converter );
        painter.restore();
        connectionIterator++;
    }

#ifdef KOFFICE_RTREE_DEBUG
    // paint tree
    double zx = 0;
    double zy = 0;
    converter.zoom( &zx, &zy );
    painter.save();
    painter.scale( zx, zy );
    d->tree.paint( painter );
    painter.restore();
#endif

    if(! forPrint)
        d->selection->paint( painter, converter );
}

KoShape * KoShapeManager::shapeAt( const QPointF &position, KoFlake::ShapeSelection selection, bool omitHiddenShapes )
{
    updateTree();
    QList<KoShape*> sortedShapes( d->tree.contains( position ) );
    qSort(sortedShapes.begin(), sortedShapes.end(), KoShape::compareShapeZIndex);
    KoShape *firstUnselectedShape = 0;
    for(int count = sortedShapes.count()-1; count >= 0; count--) {
        KoShape *shape = sortedShapes.at(count);
        if ( omitHiddenShapes && ! shape->isVisible() )
            continue;
        if ( ! shape->hitTest( position ) )
            continue;

        switch ( selection )
        {
            case KoFlake::ShapeOnTop:
                return shape;
            case KoFlake::Selected:
                if ( d->selection->isSelected( shape ) )
                    return shape;
                break;
            case KoFlake::Unselected:
                if ( ! d->selection->isSelected( shape ) )
                    return shape;
                break;
            case KoFlake::NextUnselected:
                // we want an unselected shape
                if ( d->selection->isSelected( shape ) )
                    continue;
                // memorize the first unselected shape
                if( ! firstUnselectedShape )
                    firstUnselectedShape = shape;
                // check if the shape above is selected
                if( count + 1 < sortedShapes.count() && d->selection->isSelected( sortedShapes.at(count + 1) ) )
                    return shape;
                break;
        }
    }
    // if we want the next unselected below a selected but there was none selected, 
    // return the first found unselected shape
    if( selection == KoFlake::NextUnselected && firstUnselectedShape )
        return firstUnselectedShape;

    if ( d->selection->hitTest( position ) )
        return d->selection;

    return 0; // missed everything
}

QList<KoShape *> KoShapeManager::shapesAt( const QRectF &rect, bool omitHiddenShapes )
{
    updateTree();
    //TODO check if object (outline) is really in the rect and we are not just
    // adding objects by their bounding rect
    if( omitHiddenShapes ) {
        QList<KoShape*> intersectedShapes( d->tree.intersects( rect ) );
        for(int count = intersectedShapes.count()-1; count >= 0; count--) {
            KoShape *shape = intersectedShapes.at( count );
            if( ! shape->isVisible() )
                intersectedShapes.removeAt( count );
        }
        return intersectedShapes;
    }
    else
        return d->tree.intersects( rect );
}

void KoShapeManager::repaint( QRectF &rect, const KoShape *shape, bool selectionHandles )
{
    d->canvas->updateCanvas( rect );
    if ( selectionHandles && d->selection->isSelected( shape ) )
    {
        if ( d->canvas->toolProxy() )
            d->canvas->toolProxy()->repaintDecorations();
    }
    if(selectionHandles) {
        foreach(KoShapeConnection *connection, shape->connections())
            d->canvas->updateCanvas( connection->boundingRect() );
    }
}

void KoShapeManager::notifyShapeChanged( KoShape * shape )
{
    Q_ASSERT(shape);
    if(d->aggregate4update.contains(shape))
        return;
    d->aggregate4update.insert( shape );
    d->shapeIndexesBeforeUpdate.insert(shape, shape->zIndex());
}

void KoShapeManager::updateTree()
{
    // for detecting collisions between shapes.
    class DetectCollision {
      public:
        DetectCollision() {}
        void detect(KoRTree<KoShape *> &tree, KoShape *s, int prevZIndex) {
            foreach(KoShape *shape, tree.intersects( s->boundingRect() )) {
                if(shape == s)
                    continue;
                if(s->zIndex() <= shape->zIndex() && prevZIndex <= shape->zIndex())
                    // Moving a shape will only make it collide with shapes below it.
                    continue;
                if(shape->collisionDetection() && !shapesWithCollisionDetection.contains(shape))
                    shapesWithCollisionDetection.append(shape);
            }
        }

        void fireSignals() {
            foreach(KoShape *shape, shapesWithCollisionDetection)
                shape->shapeChanged(KoShape::CollisionDetected);
        }

      private:
        QList<KoShape*> shapesWithCollisionDetection;
    };
    DetectCollision detector;
    foreach ( KoShape *shape, d->aggregate4update )
        detector.detect(d->tree, shape, d->shapeIndexesBeforeUpdate[shape]);

    foreach ( KoShape * shape, d->aggregate4update )
    {
        d->tree.remove( shape );
        QRectF br( shape->boundingRect() );
        d->tree.insert( br, shape );

        foreach(KoShapeConnection *connection, shape->connections()) {
            d->connectionTree.remove(connection);
            d->connectionTree.insert(connection->boundingRect(), connection);
        }
    }

    // do it again to see which shapes we intersect with _after_ moving.
    foreach ( KoShape *shape, d->aggregate4update )
        detector.detect(d->tree, shape, d->shapeIndexesBeforeUpdate[shape]);
    d->aggregate4update.clear();
    d->shapeIndexesBeforeUpdate.clear();

    detector.fireSignals();
}

const QList<KoShape *> & KoShapeManager::shapes() const {
    return d->shapes;
}

KoSelection * KoShapeManager::selection() const {
    return d->selection;
}

void KoShapeManager::addShapeConnection(KoShapeConnection *connection) {
    d->connectionTree.insert(connection->boundingRect(), connection);
}

#include "KoShapeManager.moc"
