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

#include <kdebug.h>

#include <KoShapeRegistry.h>
#include <KoTosContainer.h>
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
    KoTosContainer *tos = dynamic_cast<KoTosContainer *>(m_ballastRoot);
    tos->setResizeBehavior(KoTosContainer::IndependentSizes);
    m_ballastTree = dynamic_cast<KoShape*>(new TreeShape(m_ballastRoot));
    m_ballastConnector = KoShapeRegistry::instance()->value("KoConnectionShape")->createDefaultShape();
    m_ballastTree->setVisible(false);
    m_ballastConnector->setVisible(false);

    m_movable = KoShapeRegistry::instance()->value("RectangleShape")->createDefaultShape();
    m_movable->setSize(QSizeF(50,20));
    m_movable->setPosition(shape->absolutePosition(KoFlake::TopLeftCorner));

    KoShape *tmp = KoShapeRegistry::instance()->value("KoConnectionShape")->createDefaultShape();
    m_connector = dynamic_cast<KoConnectionShape*>(tmp);
    m_connector->setVisible(false);

    KoShapeController *controller = tool->canvas()->shapeController();
    m_command = new KUndo2Command;
    controller->addShapeDirect(m_movable,m_command);
    controller->addShapeDirect(m_connector,m_command);
    controller->addShapeDirect(m_ballastTree,m_command);
    controller->addShapeDirect(m_ballastConnector,m_command);
    m_command->redo();
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
        if (m_ballastTree->parent()) {
            m_ballastTree->setVisible(false);
            m_ballastConnector->setVisible(false);
            m_ballastTree->parent()->removeShape(m_ballastTree);
        }
        dynamic_cast<TreeShape*>(m_ballastTree)->setNextShape(m_newNextShape);
        kDebug() << m_newParent->shapeId() << m_newNextShape; // << m_newParent->root()->shapeId();
        m_ballastTree->setVisible(true);
        m_ballastConnector->setVisible(true);
        m_connector->setVisible(true);
        m_newParent->addChild(m_ballastTree, m_ballastConnector);
        QPair<int, int> points = chooseConnectionPoints(proposed->structure());
        m_connector->connectFirst(m_movable, points.first);
        m_connector->connectSecond(m_newParent->root(), points.second);
        m_connector->updateConnections();
    }
}

KUndo2Command* TreeShapeMoveStrategy::createCommand()
{
    if (m_diff.isNull() || m_selectedShapes.isEmpty() || (m_newParent == 0)) {
        kDebug() << "children " << m_selectedShapes.size() << "parent" << m_newParent;
        return 0;
    }
    return new TreeShapeMoveCommand(m_selectedShapes, m_newParent, m_newNextShape, m_diff);
}

void TreeShapeMoveStrategy::finishInteraction(Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED( modifiers );
    kDebug() << "";
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

//     QPointF pos = m_movable->absolutePosition(KoFlake::TopLeftCorner)+QPointF(-length, length);
//     QSizeF size = m_movable->size() + QSizeF(2*length, 2*length);
//     QRectF searchArea(pos, size);

    QList< QPair<TreeShape*, qreal> > trees;
    TreeShape *tree;
    QPointF pos, treePos;
    QRectF rect;
    qreal minDistance = 10000, distance;

    // down
    pos = m_movable->shapeToDocument(m_movable->connectionPoints().value(0).position);
    rect = QRectF(pos-QPointF(0,length), QSizeF(1,length));
    tree = propose(rect, TreeShape::OrgDown);
    if (tree) {
        treePos = tree->root()->shapeToDocument(tree->root()->connectionPoints().value(2).position);
        distance = (pos.x() - treePos.x())*(pos.x() - treePos.x())
                    + (pos.y() - treePos.y())*(pos.y() - treePos.y());
        if (distance<minDistance) {
            minDistance = distance;
            parent = tree;
        }
    }

    // up
    pos = m_movable->shapeToDocument(m_movable->connectionPoints().value(2).position);
    rect = QRectF(pos, QSizeF(1,length));
    tree = propose(rect, TreeShape::OrgUp);
    if (tree) {
        treePos = tree->root()->shapeToDocument(tree->root()->connectionPoints().value(0).position);
        distance = (pos.x() - treePos.x())*(pos.x() - treePos.x())
                    + (pos.y() - treePos.y())*(pos.y() - treePos.y());
        if (distance<minDistance) {
            minDistance = distance;
            parent = tree;
        }
    }

    // right
    pos = m_movable->shapeToDocument(m_movable->connectionPoints().value(3).position);
    rect = QRectF(pos-QPointF(length,0), QSizeF(length,1));
    tree = propose(rect, TreeShape::OrgRight);
    if (tree) {
        treePos = tree->root()->shapeToDocument(tree->root()->connectionPoints().value(1).position);
        distance = (pos.x() - treePos.x())*(pos.x() - treePos.x())
                    + (pos.y() - treePos.y())*(pos.y() - treePos.y());
        if (distance<minDistance) {
            minDistance = distance;
            parent = tree;
        }
    }

    // left
    pos = m_movable->shapeToDocument(m_movable->connectionPoints().value(1).position);
    rect = QRectF(pos, QSizeF(length,1));
    tree = propose(rect, TreeShape::OrgLeft);
    if (tree) {
        treePos = tree->root()->shapeToDocument(tree->root()->connectionPoints().value(3).position);
        distance = (pos.x() - treePos.x())*(pos.x() - treePos.x())
                    + (pos.y() - treePos.y())*(pos.y() - treePos.y());
        if (distance<minDistance) {
            minDistance = distance;
            parent = tree;
        }
    }

    return parent;
}

TreeShape* TreeShapeMoveStrategy::propose(QRectF area, TreeShape::TreeType structure)
{
    TreeShape *parent = 0;

    QList<KoShape*> shapes = tool()->canvas()->shapeManager()->shapesAt(area);
    QList<TreeShape*> trees;
    foreach (KoShape *shape, shapes) {
        TreeShape *tree = dynamic_cast<TreeShape*>(shape);
        if (tree) kDebug() << tree->structure();
        if (tree && (tree->structure() == structure))
            trees.append(tree);
    }
    trees.removeOne(dynamic_cast<TreeShape*>(m_ballastTree));
    foreach(KoShape *shape, m_selectedShapes)
        trees.removeOne(dynamic_cast<TreeShape*>(shape));

    if (!trees.isEmpty()) {
        qSort(trees.begin(), trees.end(), KoShape::compareShapeZIndex);

        kDebug() << trees.count();
        foreach (TreeShape *tree, trees)
            kDebug() << tree->shapeId() << tree->zIndex();

        parent = trees.last();
    }

    return parent;
}

QPair< int, int > TreeShapeMoveStrategy::chooseConnectionPoints(TreeShape::TreeType structure)
{
    QPair< int, int > points;
    switch (structure) {
        case TreeShape::OrgDown:
            points.first = 0;
            points.second = 2;
            break;
        case TreeShape::OrgUp:
            points.first = 2;
            points.second = 0;
            break;
        case TreeShape::OrgLeft:
            points.first = 1;
            points.second = 3;
            break;
        case TreeShape::OrgRight:
            points.first = 3;
            points.second = 1;
            break;
        default:
            break;
    }

    return points;
}
