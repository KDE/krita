/* This file is part of the KDE project
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

#include "Tree.h"
#include "Layout.h"
#include "KoShape.h"
#include "KoShapeContainer.h"
#include "KoShapeContainerModel.h"
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

Tree::Tree(): KoShapeContainer(new Layout(this))
{
    KoShape *root = KoShapeRegistry::instance()->value("EllipseShape")->createDefaultShape();
    root->setSize(QSizeF(70,70));
    root->setParent(this);
    layout()->setRoot(root);
    for (int i=0; i<3; i++){
        addNewChild();
    }
}

Tree::Tree(KoShape *shape): KoShapeContainer(new Layout(this))
{
    shape->setParent(this);
    layout()->setRoot(shape);
}

Tree::~Tree()
{
}

void Tree::addNewChild()
{
    kDebug() << "start";
    KoShape *shape = KoShapeRegistry::instance()->value("EllipseShape")->createDefaultShape();
    shape->setSize(QSizeF(40,40));
    KoShape *child = new Tree(shape);
    addShape(child);
    layout()->layout();
    kDebug() << "end";
}

void Tree::paintComponent(QPainter &painter, const KoViewConverter &converter)
{
    kDebug() << "start";
    Q_UNUSED(painter);
    Q_UNUSED(converter);
    //will be relayouted only if needed
    layout()->layout();
    kDebug() << "end";
}

bool Tree::hitTest(const QPointF &position) const
{
    return layout()->root()->hitTest(position);
}

// void Tree::shapeChanged(ChangeType type, KoShape *shape)
// {
//     Q_UNUSED(shape);
//     Q_UNUSED(type);
//     kDebug() << "";
// }

void Tree::saveOdf(KoShapeSavingContext &context) const
{
    Q_UNUSED(context);
}

bool Tree::loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    Q_UNUSED(element);
    Q_UNUSED(context);
    return true;
}

Layout* Tree::layout() const
{
    Layout *l = dynamic_cast<Layout*>(KoShapeContainer::model());
    Q_ASSERT( l );
    return l;
}