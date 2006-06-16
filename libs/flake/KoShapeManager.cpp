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
#include "KoRepaintManager.h"
#include "KoShapeContainer.h"
#include "KoShapeBorderModel.h"
#include "KoRepaintManager.moc"

#include <QDebug>
#include <QPainter>

KoShapeManager::KoShapeManager( KoCanvasBase *canvas, const QList<KoShape *> &shapes )
: m_selection( new KoSelection() )
{
    connect( m_selection, SIGNAL(selectionChanged()), this, SIGNAL(selectionChanged()) );
    m_repaintManager = new KoRepaintManager(canvas, m_selection);
    setShapes(shapes);
    m_selection->setRepaintManager(m_repaintManager);
}

KoShapeManager::KoShapeManager(KoCanvasBase *canvas)
: m_shapes()
, m_selection( new KoSelection() )
{
    connect( m_selection, SIGNAL(selectionChanged()), this, SIGNAL(selectionChanged()) );
    m_repaintManager = new KoRepaintManager(canvas, m_selection);
    m_selection->setRepaintManager(m_repaintManager);
}


KoShapeManager::~KoShapeManager()
{
    delete m_selection;
    m_repaintManager->dismantle();
}


void KoShapeManager::setShapes( const QList<KoShape *> &shapes )
{
    m_shapes = shapes;
    foreach(KoShape *shape, m_shapes)
        shape->setRepaintManager(m_repaintManager);
}

void KoShapeManager::add(KoShape *shape) {
    shape->setRepaintManager(m_repaintManager);
    m_shapes.append(shape);
}

void KoShapeManager::remove(KoShape *shape) {
    m_shapes.removeAll(shape);
}

void KoShapeManager::paint( QPainter &painter, KoViewConverter &converter, bool forPrint)
{
    QPen pen(Qt::NoPen);  // painters by default have a black stroke, lets turn that off.
    painter.setPen(pen);
    QList<KoShape*> sorterdShapes(m_shapes);
    qSort(sorterdShapes.begin(), sorterdShapes.end(), KoShape::compareShapeZIndex);
    const QRegion clipRegion = painter.clipRegion();
    foreach ( KoShape * shape, sorterdShapes ) {
        if(! shape->isVisible())
            continue;
        if(shape->parent() != 0 && shape->parent()->childClipped(shape))
            continue;
        if(painter.hasClipping()) {
            QRectF shapeBox = shape->boundingRect();
            shapeBox = converter.normalToView(shapeBox);
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

    if(! forPrint)
        m_selection->paint( painter, converter );
}

KoShape * KoShapeManager::getShapeAt( const QPointF &position )
{
    QList<KoShape*> sorterdShapes(m_shapes);
    qSort(sorterdShapes.begin(), sorterdShapes.end(), KoShape::compareShapeZIndex);
    for(int count = sorterdShapes.count()-1; count >= 0; count--) {
        if ( sorterdShapes.at(count)->hitTest( position ) )
        {
            //qDebug() << "Hittest succeeded";
            return sorterdShapes.at(count);
        }
    }
    if ( m_selection->hitTest( position ) )
        return m_selection;

    return 0; // missed everything
}

#include "KoShapeManager.moc"
