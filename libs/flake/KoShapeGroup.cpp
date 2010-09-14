/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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

#include "KoShapeGroup.h"
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

#include <QPainter>

KoShapeGroup::KoShapeGroup()
        : KoShapeContainer(new SimpleShapeContainerModel())
{
    setSize(QSizeF(0, 0));
}

KoShapeGroup::~KoShapeGroup()
{
}

void KoShapeGroup::paintComponent(QPainter &painter, const KoViewConverter &converter)
{
    Q_UNUSED(painter);
    Q_UNUSED(converter);
}

bool KoShapeGroup::hitTest(const QPointF &position) const
{
    Q_UNUSED(position);
    return false;
}

QSizeF KoShapeGroup::size() const
{
    return QSizeF(0, 0);
}

void KoShapeGroup::shapeCountChanged()
{
    // TODO: why is this needed here ? the group/ungroup command should take care of this
    QRectF br = boundingRect();
    setAbsolutePosition(br.topLeft(), KoFlake::TopLeftCorner);
    setSize(br.size());
}

void KoShapeGroup::saveOdf(KoShapeSavingContext & context) const
{
    context.xmlWriter().startElement("draw:g");
    saveOdfAttributes(context, (OdfMandatories ^ OdfLayer) | OdfAdditionalAttributes);
    context.xmlWriter().addAttributePt("svg:y", position().y());

    QList<KoShape*> shapes = this->shapes();
    qSort(shapes.begin(), shapes.end(), KoShape::compareShapeZIndex);

    foreach(KoShape* shape, shapes) {
        shape->saveOdf(context);
    }

    saveOdfCommonChildElements(context);
    context.xmlWriter().endElement();
}

bool KoShapeGroup::loadOdf(const KoXmlElement & element, KoShapeLoadingContext &context)
{
    loadOdfAttributes(element, context, OdfMandatories | OdfAdditionalAttributes | OdfCommonChildElements);

    KoXmlElement child;
    QMap<KoShapeLayer*, int> usedLayers;
    forEachElement(child, element) {
        KoShape * shape = KoShapeRegistry::instance()->createShapeFromOdf(child, context);
        if (shape) {
            KoShapeLayer *layer = dynamic_cast<KoShapeLayer*>(shape->parent());
            if (layer) {
                usedLayers[layer]++;
            }
            addShape(shape);
        }
    }
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

    QRectF bound;
    bool boundInitialized = false;
    foreach(KoShape * shape, shapes()) {
        if (! boundInitialized) {
            bound = shape->boundingRect();
            boundInitialized = true;
        } else
            bound = bound.united(shape->boundingRect());
    }

    setSize(bound.size());
    setPosition(bound.topLeft());

    foreach(KoShape * shape, shapes())
        shape->setAbsolutePosition(shape->absolutePosition() - bound.topLeft());

    return true;
}

void KoShapeGroup::shapeChanged(ChangeType type, KoShape *shape)
{
    Q_UNUSED(shape);
    switch (type) {
    case KoShape::BorderChanged:
    {
        KoShapeBorderModel *stroke = border();
        if (stroke) {
            if (stroke->deref())
                delete stroke;
            setBorder(0);
        }
        break;
    }
    case KoShape::ShadowChanged:
    {
        KoShapeShadow *shade = shadow();
        if (shade) {
            if (shade->deref())
                delete shade;
            setShadow(0);
        }
        break;
    }
    default:
        break;
    }
}
