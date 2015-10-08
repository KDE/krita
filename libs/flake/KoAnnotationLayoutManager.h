/* This file is part of the KDE project
 * Copyright (C) 2013 Mojtaba Shahi Senobari <mojtaba.shahi3000@gmail.com>
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
#ifndef KOANNOTATIONLAYOUTMANAGER_H
#define KOANNOTATIONLAYOUTMANAGER_H

#include "kritaflake_export.h"

#include <QObject>

class KoShapeManager;
class KoCanvasBase;

class QPainter;
class QPointF;

class KoShape;

class KRITAFLAKE_EXPORT KoAnnotationLayoutManager: public QObject
{
    Q_OBJECT
public:
    static const qreal shapeSpace; // Distance between annotation shapes.
    static const qreal shapeWidth; // Annotation shapes width.
    //Connection point of lines from shape to this point and from this point to refText psoition.
    static const qreal connectionPointLines;

    explicit KoAnnotationLayoutManager(QObject *parent = 0);
    virtual ~KoAnnotationLayoutManager();

    void setShapeManager(KoShapeManager *shapeManager);

    void setCanvasBase(KoCanvasBase *canvas);

    void setViewContentWidth(qreal width);

    void paintConnections(QPainter &painter);
    // Return true if shape is in annotaion shapes list.
    bool isAnnotationShape(KoShape *shape);

public Q_SLOTS:
    /// register the position of an annotation shape.
    void registerAnnotationRefPosition(KoShape *annotationShape, const QPointF& refPosition);

    /// Remove annotation shape.
    void removeAnnotationShape(KoShape *annotationShape);

    /// Update layout annotation shapes. Called when shape size changed.
    void updateLayout(KoShape *shape);

Q_SIGNALS:
    void hasAnnotationsChanged(bool hasAnnotations);

private:
    /// layout annotation shapes
    void layoutAnnotationShapes();


private:
    class Private;
    Private * const d;
};

#endif // KOANNOTATIONLAYOUTMANAGER_H

