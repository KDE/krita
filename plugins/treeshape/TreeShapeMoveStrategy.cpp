/* This file is part of the KDE project

   Copyright (c) 2010 Cyril Oblikov <munknex@gmail.com>

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

#include "TreeShape.h"
#include "TreeShapeMoveStrategy.h"
#include "TreeShapeMoveCommand.h"
#include "SelectionDecorator.h"

#include "KoShapeRegistry.h"
#include "kdebug.h"

#include <KoCanvasBase.h>
#include <KoShapeManager.h>
#include <KoShapeContainer.h>
#include <KoShapeContainerModel.h>
#include <KoResourceManager.h>
#include <KoSnapGuide.h>
#include <KoPointerEvent.h>
#include <KoToolBase.h>
#include <KLocale>
#include <KoConnectionShape.h>
#include <KoShapeController.h>
#include <KoGradientBackground.h>

TreeShapeMoveStrategy::TreeShapeMoveStrategy(KoToolBase *tool, const QPointF &clicked)
    : KoInteractionStrategy(tool),
    m_start(clicked),
    m_newParent(0),
    m_newNextShape(0)
{
    KoShape *shape = tool->canvas()->shapeManager()->shapeAt(clicked);
    m_initialOffset = shape->absolutePosition(KoFlake::TopLeftCorner) - clicked;

    m_ballastRoot = KoShapeRegistry::instance()->value("RectangleShape")->createDefaultShape();
    m_ballastRoot->setSize(QSizeF(50,20));
    QRadialGradient *gradient = new QRadialGradient(QPointF(0.5,0.5), 0.5, QPointF(0.25,0.25));
    gradient->setCoordinateMode(QGradient::ObjectBoundingMode);
    gradient->setColorAt(0.0, Qt::white);
    gradient->setColorAt(1.0, Qt::blue);
    m_ballastRoot->setBackground(new KoGradientBackground(gradient));
    m_ballastTree = dynamic_cast<KoShape*>(new TreeShape(m_ballastRoot));
    m_ballastConnector = KoShapeRegistry::instance()->value("KoConnectionShape")->createDefaultShape();

    m_movable = KoShapeRegistry::instance()->value("RectangleShape")->createDefaultShape();
    m_movable->setSize(QSizeF(50,20));
    m_movable->setPosition(shape->absolutePosition(KoFlake::TopLeftCorner));

    KoShape *tmp = KoShapeRegistry::instance()->value("KoConnectionShape")->createDefaultShape();
    m_connector = dynamic_cast<KoConnectionShape*>(tmp);

    KoShapeController *controller = tool->canvas()->shapeController();
    m_command = new QUndoCommand;
    controller->addShapeDirect(m_movable,m_command);
    controller->addShapeDirect(m_connector,m_command);
    controller->addShapeDirect(m_ballastTree,m_command);
    controller->addShapeDirect(m_ballastConnector,m_command);
    m_command->redo();
//     tool->canvas()->shapeManager()->addShape(m_movable);
//     tool->canvas()->shapeManager()->addShape(m_connector);
//     tool->canvas()->shapeManager()->addShape(m_ballast);
//     tool->canvas()->shapeManager()->addShape(m_ballastConnector);
    m_connector->setZIndex(shape->zIndex()+5);
    m_movable->setZIndex(shape->zIndex()+5);
    m_connector->updateConnections();
    QList<KoShape*> selectedShapes = tool->canvas()->shapeManager()->selection()->selectedShapes(KoFlake::TopLevelSelection);
    foreach(KoShape *shape, selectedShapes) {
        kDebug() << "selected shape" << shape
                 << "to tree" << dynamic_cast<TreeShape*>(shape);
        if (dynamic_cast<TreeShape*>(shape->parent())) {
            kDebug() << "parent" << shape->parent()
                     << "to tree" << dynamic_cast<TreeShape*>(shape->parent());
            m_selectedShapes << shape->parent();
        }
    }
}

void TreeShapeMoveStrategy::handleMouseMove(const QPointF &point, Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers);
    if(m_selectedShapes.isEmpty())
        return;

    m_diff = point - m_start;
    m_movable->update();
    m_movable->setPosition(point+m_initialOffset);
    m_movable->update();

    TreeShape *proposed = proposeParent();
    if (proposed && (proposed != m_newParent)) {
        m_newParent = proposed;
        m_newNextShape = proposed->proposePosition(m_movable);
        //dynamic_cast<TreeShape*>(m_ballastTree)->setNextShape(m_newNextShape);
        //m_newParent->addChild(m_ballastTree, m_ballastConnector);
        //kDebug() << "|||" << m_newParent->shapeId();// << m_newParent->root()->shapeId();
        m_connector->connectFirst(m_movable, 0);
        m_connector->connectSecond(m_newParent->root(),2);
        m_connector->updateConnections();
    }
}

QUndoCommand* TreeShapeMoveStrategy::createCommand()
{
    if (m_diff.isNull() || m_selectedShapes.isEmpty() || (m_newParent == 0)) {
        kDebug() << "children " << m_selectedShapes.size() << "parent" << m_newParent;
        return 0;
    }
    return new TreeShapeMoveCommand(m_selectedShapes, m_newParent, m_newNextShape);
}

void TreeShapeMoveStrategy::finishInteraction(Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED( modifiers );
    m_command->undo();
}

void TreeShapeMoveStrategy::paint( QPainter &painter, const KoViewConverter &converter)
{
    SelectionDecorator decorator;
    decorator.setSelection(tool()->canvas()->shapeManager()->selection());
    decorator.paint(painter, converter);
    m_connector->paint(painter, converter);
}

TreeShape* TreeShapeMoveStrategy::proposeParent()
{
    TreeShape *parent = 0;
    qreal length = 50;

    QPointF cp = m_movable->absoluteTransformation(0).map(m_movable->connectionPoints()[0]);
    QRectF rect = QRectF(cp-QPointF(0,length), QSizeF(1,length));
    QList<KoShape*> shapes = tool()->canvas()->shapeManager()->shapesAt(rect);
    QList<TreeShape*> trees;
    foreach (KoShape *shape, shapes) {
        TreeShape *tree = dynamic_cast<TreeShape*>(shape);
        if (tree && !tree->root()->boundingRect().contains(cp))
            trees.append(tree);
    }

    if (!trees.isEmpty()) {
        qSort(trees.begin(), trees.end(), KoShape::compareShapeZIndex);

        kDebug() << trees.count();
        foreach (TreeShape *tree, trees)
            kDebug() << tree->shapeId() << tree->zIndex();

        parent = trees.last();
    }

    return parent;
}
