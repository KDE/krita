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
#include "Layout.h"
#include "KoShape.h"
#include "KoShapeContainer.h"
#include "KoShapeContainerModel.h"
#include "KoConnectionShape.h"
#include "KoShapeLayer.h"
#include "SimpleShapeContainerModel.h"
#include "KoShapeSavingContext.h"
#include "KoShapeLoadingContext.h"
#include "KoXmlWriter.h"
#include "KoXmlReader.h"
#include "KoShapeRegistry.h"
#include "KoShapeBorderModel.h"
#include "KoShapeShadow.h"

#include <KoShapeRegistry.h>

#include <QPainter>

#include "kdebug.h"

TreeShape::TreeShape(): KoShapeContainer(new Layout(this))
{
    m_nextShape = 0;
    KoShape *root = KoShapeRegistry::instance()->value("RectangleShape")->createDefaultShape();
    root->setShapeId("Ellipse000");
    root->setSize(QSizeF(60,30));
    root->setParent(this);
    layout()->setRoot(root);
    for (int i=0; i<3; i++) {
        addNewChild();
    }
}

TreeShape::TreeShape(KoShape *shape): KoShapeContainer(new Layout(this))
{
    m_nextShape = 0;
    int id = qrand()%90+10;
    setShapeId("TreeShape"+QString::number(id));
    shape->setShapeId("Ellipse"+QString::number(id));
    addShape(shape);
    layout()->setRoot(shape);
    layout()->layout();
    update();
}

TreeShape::~TreeShape()
{
}

void TreeShape::setZIndex(int zIndex)
{
    KoShape::setZIndex(zIndex);
    layout()->root()->setZIndex(zIndex+1);
}

void TreeShape::paintComponent(QPainter &painter, const KoViewConverter &converter)
{
    //kDebug() << "start" << this->shapeId();
    Q_UNUSED(painter);
    Q_UNUSED(converter);
    //will be relayouted only if needed
    layout()->layout();
    //kDebug() << "end" << this->shapeId();
}

bool TreeShape::hitTest(const QPointF &position) const
{
    Q_UNUSED(position);
    //return layout()->root()->hitTest(position);
    //kDebug() << shapeId();
    return false;
}

// void TreeShape::shapeChanged(ChangeType type, KoShape *shape)
// {
//     Q_UNUSED(shape);
//     Q_UNUSED(type);
//     kDebug() << "";
// }

void TreeShape::saveOdf(KoShapeSavingContext &context) const
{
    Q_UNUSED(context);
}

bool TreeShape::loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    Q_UNUSED(element);
    Q_UNUSED(context);
    return true;
}

void TreeShape::addChild(KoShape* tree, KoShape* connector)
{
    addShape(tree);
    addShape(connector);
    layout()->attachConnector(tree, dynamic_cast<KoConnectionShape*>(connector));
    layout()->layout();

//     TreeShape *p = dynamic_cast<TreeShape*>(parent());
//     while (p) {
//         kDebug() << p->shapeId();
//         p->update();
//         p = dynamic_cast<TreeShape*>(p->parent());
//     }

}

KoShape* TreeShape::connector(KoShape *shape)
{
    Layout *layout = dynamic_cast<Layout*>(KoShapeContainer::model());
    Q_ASSERT(layout);
    return layout->connector(shape);
}

KoShape* TreeShape::root() const
{
    Layout *layout = dynamic_cast<Layout*>(KoShapeContainer::model());
    Q_ASSERT(layout);
    return layout->root();
}

void TreeShape::setStructure(TreeShape::TreeType structure)
{
    Layout *layout = dynamic_cast<Layout*>(KoShapeContainer::model());
    Q_ASSERT(layout);
    kDebug() << "";
    layout->setStructure(structure);
}

TreeShape::TreeType TreeShape::structure() const
{
    Layout *layout = dynamic_cast<Layout*>(KoShapeContainer::model());
    Q_ASSERT(layout);
    return layout->structure();
}

QList<KoShape*> TreeShape::addNewChild()
{
    kDebug() << "start";
    QList<KoShape*> shapes;

    KoShape *root = KoShapeRegistry::instance()->value("RectangleShape")->createDefaultShape();
    root->setSize(QSizeF(50,20));
    KoShape *child = new TreeShape(root);
    shapes.append(child);

    KoShape *connector = KoShapeRegistry::instance()->value("KoConnectionShape")->createDefaultShape();
    shapes.append(connector);

    addChild(child, connector);
    kDebug() << "end";
    return shapes;
}

void TreeShape::setNextShape(KoShape* shape)
{
    m_nextShape = shape;
}

KoShape* TreeShape::nextShape()
{
    return m_nextShape;
}

KoShape* TreeShape::proposePosition(KoShape* shape)
{
    return layout()->proposePosition(shape);
}

Layout* TreeShape::layout() const
{
    Layout *l = dynamic_cast<Layout*>(KoShapeContainer::model());
    Q_ASSERT(l);
    return l;
}
