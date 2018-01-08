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
