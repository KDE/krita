/*
 *  SPDX-FileCopyrightText: 2009 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_shape_tool_helper.h"

#include <QPainterPath>

#include <KoPathShape.h>
#include <KoShapeRegistry.h>
#include <KoShapeFactoryBase.h>
#include <KoProperties.h>


KoShape* KisShapeToolHelper::createRectangleShape(const QRectF& rect, qreal roundCornersX, qreal roundCornersY)
{
    KoShape* shape;

    KoShapeFactoryBase *rectFactory = KoShapeRegistry::instance()->value("RectangleShape");
    if (rectFactory) {
        KoProperties props;
        props.setProperty("x", rect.x());
        props.setProperty("y", rect.y());
        props.setProperty("width", rect.width());
        props.setProperty("height", rect.height());
        props.setProperty("rx", 2 * 100.0 * roundCornersX / rect.width());
        props.setProperty("ry", 2 * 100.0 * roundCornersY / rect.height());

        shape = rectFactory->createShape(&props);
    } else {
        //Fallback if the plugin wasn't found
        QPainterPath path;
        if (roundCornersX > 0 || roundCornersY > 0) {
            path.addRoundedRect(rect, roundCornersX, roundCornersY);
        } else {
            path.addRect(rect);
        }
        KoPathShape *pathShape = KoPathShape::createShapeFromPainterPath(path);
        pathShape->normalize();
        shape = pathShape;
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

