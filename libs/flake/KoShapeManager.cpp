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
#include "KoCanvasBase.h"
#include "KoShapeContainer.h"
#include "KoShapeBorderModel.h"
#include "KoShapeGroup.h"
#include "KoTool.h"

#include <QPainter>

KoShapeManager::KoShapeManager( KoCanvasBase *canvas, const QList<KoShape *> &shapes )
: m_selection( new KoSelection() )
, m_canvas( canvas )    
, m_tree( 4, 2 )
{
    connect( m_selection, SIGNAL(selectionChanged()), this, SIGNAL(selectionChanged()) );
    setShapes(shapes);
    m_selection->addShapeManager( this );
}

KoShapeManager::KoShapeManager(KoCanvasBase *canvas)
: m_shapes()
, m_selection( new KoSelection() )
, m_canvas( canvas )    
, m_tree( 4, 2 )
{
    connect( m_selection, SIGNAL(selectionChanged()), this, SIGNAL(selectionChanged()) );
    m_selection->addShapeManager( this );
}


KoShapeManager::~KoShapeManager()
{
    foreach(KoShape *shape, m_shapes)
        shape->removeShapeManager( this );
    delete m_selection;
}


void KoShapeManager::setShapes( const QList<KoShape *> &shapes )
{
    foreach(KoShape *shape, m_shapes)
    {
        m_aggregate4update.remove( shape );
        m_tree.remove( shape );
        shape->removeShapeManager( this );
    }
    m_shapes = shapes;
    foreach(KoShape *shape, m_shapes)
    {
        add( shape );
    }
}

void KoShapeManager::add( KoShape *shape )
{
    shape->addShapeManager( this );
    m_shapes.append(shape);
    if( ! dynamic_cast<KoShapeGroup*>( shape ))
    {
        QRectF br( shape->boundingRect() );
        m_tree.insert( shape, br );
    }
    shape->repaint();
}

void KoShapeManager::remove( KoShape *shape )
{
    shape->removeShapeManager( this );
    m_aggregate4update.remove( shape );
    m_tree.remove( shape );
    m_shapes.removeAll(shape);
}

void KoShapeManager::paint( QPainter &painter, KoViewConverter &converter, bool forPrint)
{
    updateTree();
    QPen pen(Qt::NoPen);  // painters by default have a black stroke, lets turn that off.
    painter.setPen(pen);
    QList<KoShape*> sorterdShapes( m_tree.intersects( converter.viewToDocument( painter.clipRegion().boundingRect() ) ) );
    qSort(sorterdShapes.begin(), sorterdShapes.end(), KoShape::compareShapeZIndex);
    const QRegion clipRegion = painter.clipRegion();

    foreach ( KoShape * shape, sorterdShapes ) {
        if(! shape->isVisible())
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
            shape->paintDecorations( painter, converter, m_selection->isSelected(shape) );
            painter.restore();
        }
        painter.restore();  // for the matrix
    }

#if 0
    // paint tree
    double zx = 0;
    double zy = 0;
    converter.zoom( &zx, &zy );
    painter.save();
    painter.scale( zx, zy );
    m_tree.paint( painter );
    painter.restore();
#endif

    if(! forPrint)
        m_selection->paint( painter, converter );
}

KoShape * KoShapeManager::shapeAt( const QPointF &position )
{
    updateTree();
    QList<KoShape*> sorterdShapes( m_tree.contains( position ) );
    qSort(sorterdShapes.begin(), sorterdShapes.end(), KoShape::compareShapeZIndex);
    for(int count = sorterdShapes.count()-1; count >= 0; count--) {
        if ( sorterdShapes.at(count)->hitTest( position ) )
        {
            //kDebug() << "Hittest succeeded" << endl;
            return sorterdShapes.at(count);
        }
    }
    if ( m_selection->hitTest( position ) )
        return m_selection;

    return 0; // missed everything
}

QList<KoShape *> KoShapeManager::shapesAt( const QRectF &rect )
{
    updateTree();
    //TODO check if object is really in the rect and not 
    // only the bounding rect of the object.
    return m_tree.intersects( rect );
}

void KoShapeManager::repaint( QRectF &rect, const KoShape *shape, bool selectionHandles )
{
    m_canvas->updateCanvas( rect );
    if ( selectionHandles && m_selection->isSelected( shape ) )
    {
        if ( m_canvas->tool() )
            m_canvas->tool()->repaintDecorations();
    }
}

void KoShapeManager::updateTree( KoShape * shape )
{
    m_aggregate4update.insert( shape );
}

void KoShapeManager::updateTree()
{
    foreach ( KoShape * shape, m_aggregate4update )
    {
        m_tree.remove( shape );
        QRectF br( shape->boundingRect() );
        m_tree.insert( shape, br );
    }
    m_aggregate4update.clear();
}

#include "KoShapeManager.moc"
