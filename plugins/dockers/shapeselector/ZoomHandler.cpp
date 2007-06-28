/*
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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

#include "ZoomHandler.h"
//   #include "shapeselector/GroupShape.h"
//   #include "shapeselector/IconShape.h"
//   #include "shapeselector/TemplateShape.h"
//
//   #include <KoShapeManager.h>
//   #include <KoPointerEvent.h>
//   #include <KoShapeRegistry.h>
//   #include <KoToolManager.h>
//   #include <KoShapeFactory.h>
//   #include <KoShape.h>
//   #include <KoShapeContainer.h>
//   #include <KoInteractionTool.h>
//   #include <KoShapeMoveStrategy.h>
//   #include <KoCreateShapesTool.h>
//   #include <KoShapeController.h>
//   #include <KoCanvasController.h>
//   #include <KoProperties.h>
//
//   #include <QKeyEvent>
//   #include <QPainter>
//   #include <QMouseEvent>
//   #include <QHelpEvent>
//   #include <QPointF>
//   #include <QToolTip>
//   #include <QTimer>
//   #include <QPainterPath>
//   #include <QUndoCommand>
//
//   #include <kdebug.h>
//   #include <kicon.h>
//   #include <klocale.h>

QPointF ZoomHandler::documentToView (const QPointF &documentPoint) const {
    return documentPoint;
}

QPointF ZoomHandler::viewToDocument (const QPointF &viewPoint) const {
    return viewPoint;
}

QRectF ZoomHandler::documentToView (const QRectF &documentRect) const {
    return documentRect;
}

QRectF ZoomHandler::viewToDocument (const QRectF &viewRect) const {
    return viewRect;
}

QSizeF ZoomHandler::documentToView (const QSizeF &documentSize) const {
    return documentSize;
}

QSizeF ZoomHandler::viewToDocument (const QSizeF &viewSize) const {
    return viewSize;
}

void ZoomHandler::zoom (double *zoomX, double *zoomY) const {
    *zoomX = 1.0;
    *zoomY = 1.0;
}

double ZoomHandler::documentToViewX (double documentX) const {
    return documentX;
}

double ZoomHandler::documentToViewY (double documentY) const {
    return documentY;
}

double ZoomHandler::viewToDocumentX (double viewX) const {
    return viewX;
}

double ZoomHandler::viewToDocumentY (double viewY) const {
    return viewY;
}
