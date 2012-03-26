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
#include "TreeLayout.h"
#include "KoShape.h"
#include "KoShapeContainer.h"
#include "KoConnectionShape.h"
#include "KoTosContainer.h"
#include "KoShapeLayer.h"
#include "KoShapeSavingContext.h"
#include "KoShapeLoadingContext.h"
#include "KoXmlWriter.h"
#include "KoXmlReader.h"
#include "KoShapeRegistry.h"
//#include "KoShapeStrokeModel.h"
#include "KoShapeShadow.h"
#include <KoShapeRegistry.h>
#include <KoXmlNS.h>

#include <QPainter>

#include "kdebug.h"

TreeShape::TreeShape(KoDocumentResourceManager *documentResources)
            : KoShapeContainer(new TreeLayout(this))
            , m_documentResources(documentResources)
{
    m_nextShape = 0;
    setName("TreeShape0");
    KoShape *root = KoShapeRegistry::instance()->value("RectangleShape")->createDefaultShape();
    root->setSize(QSizeF(60,30));
    KoTosContainer *tosContainer = dynamic_cast<KoTosContainer*>(root);
    tosContainer->setResizeBehavior(KoTosContainer::IndependentSizes);
    // we need to create the text shape first before we can set the text to it
    // There is no text so it should not be needed to set it
    // tos->setPlainText(" ");
    root->setName("TextOnShape0");
    root->setParent(this);
    layout()->setRoot(root, Rectangle);
    setStructure(OrgDown);
    for (int i=0; i<0; i++) {
        addNewChild();
    }
}

TreeShape::TreeShape(KoShape *shape, KoDocumentResourceManager *documentResources)
            : KoShapeContainer(new TreeLayout(this))
            , m_documentResources(documentResources)
{
    m_nextShape = 0;
    int id = qrand()%90+10;
    setShapeId(TREESHAPEID);
    setName(TREESHAPEID+QString::number(id));
    shape->setName("TextOnShape"+QString::number(id));
    addShape(shape);
    layout()->setRoot(shape, Rectangle);
    layout()->layout();
    setStructure(OrgDown);
    update();
}

TreeShape::~TreeShape()
{
    kDebug() << "";
    KoShape *tos = root();
    if (tos) {
        kDebug() << this << "deleting root";
        delete tos;
    }
}

void TreeShape::setZIndex(int zIndex)
{
    KoShape::setZIndex(zIndex);
}

void TreeShape::paintComponent(QPainter &painter, const KoViewConverter &converter, KoShapePaintingContext &)
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
    kDebug() << layout()->root()->hitTest(position);
    return layout()->root()->hitTest(position);
    //Q_UNUSED(position);
    //kDebug() << shapeId();
    //return false;
}

// void TreeShape::shapeChanged(ChangeType type, KoShape *shape)
// {
//     KoShapeContainer::shapeChanged(type, shape);
//     kDebug() << "";
// }

void TreeShape::saveOdf(KoShapeSavingContext &context) const
{
    //Q_UNUSED(context);
    context.xmlWriter().startElement("draw:tree");
    saveOdfAttributes(context, (OdfMandatories ^ OdfLayer) | OdfAdditionalAttributes);
    context.xmlWriter().addAttributePt("svg:x", position().x());
    context.xmlWriter().addAttributePt("svg:y", position().y());
    context.xmlWriter().addAttribute("draw:structure", structure());
    context.xmlWriter().addAttribute("draw:rootType", rootType());
    context.xmlWriter().addAttribute("draw:connectionType", connectionType());

    QList<KoShape*> children = shapes();
    //qSort(children.begin(), children.end(), KoShape::compareShapeZIndex);

    foreach(KoShape* shape, children) {
        shape->saveOdf(context);
    }

    saveOdfCommonChildElements(context);
    context.xmlWriter().endElement();
}

bool TreeShape::loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    loadOdfAttributes(element, context, OdfMandatories | OdfAdditionalAttributes | OdfCommonChildElements);

    QString attribute;

    Q_ASSERT(element.hasAttributeNS(KoXmlNS::draw, "structure"));
    attribute = element.attributeNS(KoXmlNS::draw, "structure");
    kDebug() << attribute;
    TreeType proposedTreeType = static_cast<TreeType>(attribute.toInt());
//     switch (attribute) {
//         case "OrgDown":
//             setStructure(OrgDown);
//             break;
//         case "OrgUp":
//             setStructure(OrgUp);
//             break;
//         case "OrgRight":
//             setStructure(OrgRight);
//             break;
//         case "OrgLeft":
//             setStructure(OrgLeft);
//             break;
//         case "TreeRight":
//             setStructure(TreeRight);
//             break;
//         case "TreeLeft":
//             setStructure(TreeLeft);
//             break;
//         case "MapClockwise":
//             setStructure(MapClockwise);
//             break;
//         case "MapAntiClockwise":
//             setStructure(MapAntiClockwise);
//             break;
//         case default:
//             kDebug() << "Unsupported tree structure:" << attribute;
//             setStructure(OrgDown);
//             break;
//     }

    Q_ASSERT(element.hasAttributeNS(KoXmlNS::draw, "rootType"));
    attribute = element.attributeNS(KoXmlNS::draw, "rootType");
    kDebug() << attribute;
    RootType proposedRootType = static_cast<RootType>(attribute.toInt());
//     switch (attribute) {
//         case "Rectangle":
//             proposedRootType = Rectangle;
//             break;
//         case "Ellipse":
//             proposedRootType = Ellipse;
//             break;
//         case "None":
//             proposedRootType = None;
//             break;
//         case default:
//             kDebug() << "Unsupported root type:" << attribute;
//             proposedRootType = Rectangle;
//             break;
//     }

    Q_ASSERT(element.hasAttributeNS(KoXmlNS::draw, "connectionType"));
    attribute = element.attributeNS(KoXmlNS::draw, "connectionType");
    kDebug() << attribute;
    KoConnectionShape::Type proposedConnectionType = static_cast<KoConnectionShape::Type>(attribute.toInt());
//     switch (attribute) {
//         case "Standart":
//             break;
//         case "Lines":
//             break;
//         case "Straight":
//             break;
//         case "Curve":
//             break;
//         case default:
//             break;
//     }

    KoXmlElement child;
    QMap<KoShapeLayer*, int> usedLayers;
    QList<KoShape*> trees, connectors;
    forEachElement(child, element) {
        KoShape *shape = KoShapeRegistry::instance()->createShapeFromOdf(child, context);
        if (shape) {
            KoShapeLayer *layer = dynamic_cast<KoShapeLayer*>(shape->parent());
            if (layer)
                usedLayers[layer]++;
            KoTosContainer *tos = dynamic_cast<KoTosContainer*>(shape);
            TreeShape *tree = dynamic_cast<TreeShape*>(shape);
            KoConnectionShape *connector = dynamic_cast<KoConnectionShape*>(shape);
            if (tos) {
                kDebug() << this << "Setting Root";
                KoShape *oldRoot = root();
                if (oldRoot)
                    oldRoot->deleteLater();
                setRoot(tos, proposedRootType);
            } else if (tree) {
                trees.append(shape);
            } else if (connector) {
                connectors.append(connector);
            } else {
                kDebug() << "Ignoring unexpectad shape:" << shape->shapeId();
            }
        }
    }

    kDebug() << "Adding children";
    for (int i=0; i<trees.count(); i++)
        addChild(trees[i], connectors[i]);

    KoShapeLayer *parent = 0;
    int maxUseCount = 0;
    // find most used layer and use this as parent for the group
    for (QMap<KoShapeLayer*, int>::const_iterator it(usedLayers.constBegin()); it != usedLayers.constEnd(); ++it) {
        if (it.value() > maxUseCount) {
            maxUseCount = it.value();
            parent = it.key();
        }
    }
    setParent(parent);

    QPointF pos;
    if (element.hasAttributeNS(KoXmlNS::svg, "x") && element.hasAttributeNS(KoXmlNS::svg, "y")) {
        pos.setX(KoUnit::parseValue(element.attributeNS(KoXmlNS::svg, "x", QString())));
        pos.setY(KoUnit::parseValue(element.attributeNS(KoXmlNS::svg, "y", QString())));
    } else {
        kDebug() << "Coordinates were missed";
    }
    setPosition(pos);
    setStructure(proposedTreeType);
    setConnectionType(proposedConnectionType);

//     QRectF bound;
//     bool boundInitialized = false;
//     foreach(KoShape * shape, shapes()) {
//         if (!boundInitialized) {
//             bound = shape->boundingRect();
//             boundInitialized = true;
//         } else
//             bound = bound.united(shape->boundingRect());
//     }
//
//     kDebug() << bound;
//     setPosition(bound.topLeft());

    return true;
}

void TreeShape::addChild(KoShape* tree, KoShape* connector)
{
    addShape(tree);
    addShape(connector);
    layout()->attachConnector(tree, dynamic_cast<KoConnectionShape*>(connector));
    layout()->layout();
}

KoShape* TreeShape::connector(KoShape *shape)
{
    TreeLayout *layout = dynamic_cast<TreeLayout*>(KoShapeContainer::model());
    Q_ASSERT(layout);
    return layout->connector(shape);
}

void TreeShape::setRoot(KoShape* shape, TreeShape::RootType type)
{
    shape->setName(name()+QString("_Root"));
    addShape(shape);
    layout()->setRoot(shape, type);
}

KoShape* TreeShape::root() const
{
    TreeLayout *layout = dynamic_cast<TreeLayout*>(KoShapeContainer::model());
    Q_ASSERT(layout);
    return layout->root();
}

TreeShape::RootType TreeShape::rootType() const
{
    return layout()->rootType();
}

void TreeShape::setStructure(TreeShape::TreeType structure)
{
    TreeLayout *layout = dynamic_cast<TreeLayout*>(KoShapeContainer::model());
    Q_ASSERT(layout);
    kDebug() << "";
    layout->setStructure(structure);
}

TreeShape::TreeType TreeShape::structure() const
{
    TreeLayout *layout = dynamic_cast<TreeLayout*>(KoShapeContainer::model());
    Q_ASSERT(layout);
    return layout->structure();
}

void TreeShape::setConnectionType(KoConnectionShape::Type type)
{
    layout()->setConnectionType(type);
}

KoConnectionShape::Type TreeShape::connectionType() const
{
    return layout()->connectionType();
}

QList<KoShape*> TreeShape::addNewChild()
{
    kDebug() << "start";
    // this list of shapes would be returned
    // used by TreeTool to add shapes to canvas
    QList<KoShape*> shapes;

    KoShape *root = KoShapeRegistry::instance()->value("RectangleShape")->createDefaultShape();
    root->setSize(QSizeF(50,20));
    KoTosContainer *tos = dynamic_cast<KoTosContainer*>(root);
    tos->setResizeBehavior(KoTosContainer::IndependentSizes);
    // we need to create the text shape first before we can set the text to it
    // There is no text so it should not be needed to set it
    // tos->setPlainText(" ");
    root = tos;
    KoShape *child = new TreeShape(root, m_documentResources);
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

TreeShape::TreeType TreeShape::proposeStructure()
{
    return layout()->proposeStructure();
}

TreeLayout* TreeShape::layout() const
{
    TreeLayout *l = dynamic_cast<TreeLayout*>(KoShapeContainer::model());
    Q_ASSERT(l);
    return l;
}
