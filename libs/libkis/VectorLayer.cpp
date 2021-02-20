/*
 *  SPDX-FileCopyrightText: 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
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
