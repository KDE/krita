/* This file is part of the KDE project
   Copyright (C) 2006-2007 Jan Hambrecht <jaham@gmx.net>

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

#include "KoShapeLayer.h"
#include "SimpleShapeContainerModel.h"
#include "KoShapeSavingContext.h"
#include "KoShapeLoadingContext.h"
#include <KoXmlReader.h>
#include <KoXmlWriter.h>
#include <KoGenStyle.h>
#include <KoGenStyles.h>
#include <KoXmlNS.h>

KoShapeLayer::KoShapeLayer()
        : KoShapeContainer(new SimpleShapeContainerModel())
{
    setSelectable(false);
}

KoShapeLayer::KoShapeLayer(KoShapeContainerModel *model)
        : KoShapeContainer(model)
{
    setSelectable(false);
}

bool KoShapeLayer::hitTest(const QPointF &position) const
{
    Q_UNUSED(position);
    return false;
}

QRectF KoShapeLayer::boundingRect() const
{
    QRectF bb;

    foreach(KoShape* shape, shapes()) {
        if (bb.isEmpty())
            bb = shape->boundingRect();
        else
            bb = bb.unite(shape->boundingRect());
    }

    return bb;
}

void KoShapeLayer::saveOdf(KoShapeSavingContext & context) const
{
    QList<KoShape*> shapes = this->shapes();
    qSort(shapes.begin(), shapes.end(), KoShape::compareShapeZIndex);

    foreach(KoShape* shape, shapes) {
        shape->saveOdf(context);
    }
}

bool KoShapeLayer::loadOdf(const KoXmlElement & element, KoShapeLoadingContext &context)
{
    // set layer name
    setName(element.attributeNS(KoXmlNS::draw, "name"));
    // layer locking
    setGeometryProtected(element.attributeNS(KoXmlNS::draw, "protected", "false") == "true");
    // layer visibility
    setVisible(element.attributeNS(KoXmlNS::draw, "display", "false") != "none");

    // add layer by name into shape context
    context.addLayer(this, name());

    return true;
}

void KoShapeLayer::paintComponent(QPainter &, const KoViewConverter &)
{
}
