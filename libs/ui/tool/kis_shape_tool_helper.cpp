/*
 *  Copyright (c) 2009 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_shape_tool_helper.h"

#include <KoPathShape.h>
#include <KoShapeRegistry.h>


KoShape* KisShapeToolHelper::createRectangleShape(const QRectF& rect)
{
    KoShape* shape;
    KoShapeFactoryBase *rectFactory = KoShapeRegistry::instance()->value("RectangleShape");
    if (rectFactory) {
        shape = rectFactory->createDefaultShape();
        shape->setSize(rect.size());
        shape->setPosition(rect.topLeft());
    } else {
        //Fallback if the plugin wasn't found
        KoPathShape* path = new KoPathShape();
        path->setShapeId(KoPathShapeId);
        path->moveTo(rect.topLeft());
        path->lineTo(rect.topLeft() + QPointF(rect.width(), 0));
        path->lineTo(rect.bottomRight());
        path->lineTo(rect.topLeft() + QPointF(0, rect.height()));
        path->close();
        path->normalize();
        shape = path;
    }
    return shape;
}

KoShape* KisShapeToolHelper::createEllipseShape(const QRectF& rect)
{
    KoShape* shape;
    KoShapeFactoryBase *rectFactory = KoShapeRegistry::instance()->value("EllipseShape");
    if (rectFactory) {
        shape = rectFactory->createDefaultShape();
        shape->setSize(rect.size());
        shape->setPosition(rect.topLeft());
    } else {
        //Fallback if the plugin wasn't found
        KoPathShape* path = new KoPathShape();
        path->setShapeId(KoPathShapeId);

        QPointF rightMiddle = QPointF(rect.left() + rect.width(), rect.top() + rect.height() / 2);
        path->moveTo(rightMiddle);
        path->arcTo(rect.width() / 2, rect.height() / 2, 0, 360.0);
        path->close();
        path->normalize();
        shape = path;
    }
    return shape;
}

