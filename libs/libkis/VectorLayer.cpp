/*
 *  Copyright (c) 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "VectorLayer.h"
#include <kis_shape_layer.h>
#include <kis_image.h>
#include <KoShapeGroup.h>

#include "GroupShape.h"

VectorLayer::VectorLayer(KoShapeControllerBase* shapeController, KisImageSP image, QString name, QObject *parent) :
    Node(image, new KisShapeLayer(shapeController, image, name, OPACITY_OPAQUE_U8), parent)
{

}

VectorLayer::VectorLayer(KisShapeLayerSP layer, QObject *parent):
    Node(layer->image(), layer, parent)
{

}

VectorLayer::~VectorLayer()
{

}

QString VectorLayer::type() const
{
    return "vectorlayer";
}

QList<Shape *> VectorLayer::shapes() const
{
    QList<Shape*> shapes;
    KisShapeLayerSP vector = KisShapeLayerSP(dynamic_cast<KisShapeLayer*>(this->node().data()));
    if (vector) {
        QList<KoShape*> originalShapes = vector->shapes();
        std::sort(originalShapes.begin(), originalShapes.end(), KoShape::compareShapeZIndex);
        for (int i=0; i<vector->shapeCount(); i++) {
            if (dynamic_cast<KoShapeGroup*>(originalShapes.at(i))) {
                shapes << new GroupShape(dynamic_cast<KoShapeGroup*>(originalShapes.at(i)));
            } else {
                shapes << new Shape(originalShapes.at(i));
            }
        }
    }
    return shapes;
}
