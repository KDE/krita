/*
 *  SPDX-FileCopyrightText: 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "GroupShape.h"
#include <KoShapeGroup.h>

GroupShape::GroupShape(QObject *parent) : Shape(new KoShapeGroup(), parent)
{
}

GroupShape::GroupShape(KoShapeGroup *shape, QObject *parent) :
    Shape(shape, parent)
{

}

GroupShape::~GroupShape()
{

}

QString GroupShape::type() const
{
    //Has no default KoID
    return "groupshape";
}

QList<Shape *> GroupShape::children()
{
    KoShapeGroup * group = dynamic_cast<KoShapeGroup*>(this->shape());
    QList <Shape*> shapes;
    if (group) {
        QList<KoShape*> originalShapes = group->shapes();
        std::sort(originalShapes.begin(), originalShapes.end(), KoShape::compareShapeZIndex);
        for(int i=0; i<group->shapeCount(); i++) {
            if (dynamic_cast<KoShapeGroup*>(originalShapes.at(i))) {
                shapes << new GroupShape(dynamic_cast<KoShapeGroup*>(originalShapes.at(i)));
            } else {
                shapes << new Shape(originalShapes.at(i));
            }
        }
    }
    return shapes;
}
