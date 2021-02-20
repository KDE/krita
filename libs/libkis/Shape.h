/*
 *  SPDX-FileCopyrightText: 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef LIBKIS_SHAPE_H
#define LIBKIS_SHAPE_H

#include <QObject>
#include <KoShape.h>

#include "kritalibkis_export.h"
#include "libkis.h"
#include <kis_types.h>

/**
 * @brief The Shape class
 * The shape class is a wrapper around Krita's vector objects.
 *
 * Some example code to parse through interesting information in a given vector layer with shapes.
 * @code
import sys
from krita import *

doc = Application.activeDocument()

root = doc.rootNode()

for layer in root.childNodes():
    print (str(layer.type())+" "+str(layer.name()))
    if (str(layer.type())=="vectorlayer"):
        for shape in layer.shapes():
            print(shape.name())
            print(shape.toSvg())
 * @endcode
 */
class KRITALIBKIS_EXPORT Shape : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Shape)

public:
    explicit Shape(KoShape *shape, QObject *parent = 0);
    ~Shape();
public Q_SLOTS:

    /**
     * @brief name
     * @return the name of the shape
     */
    QString name() const;

    /**
     * @brief setName
     * @param name which name the shape should have.
     */
    void setName(const QString &name);

    /**
     * @brief type
     * @return the type of shape.
     */
    virtual QString type() const;

    /**
     * @brief visible
     * @return whether the shape is visible.
     */
    bool visible();

    /**
     * @brief setVisible
     * @param visible whether the shape should be visible.
     */
    void setVisible(bool visible);

    /**
     * @brief boundingBox the bounding box of the shape in points
     * @return RectF containing the bounding box.
     */
    QRectF boundingBox() const;

    /**
     * @brief position the position of the shape in points.
     * @return the position of the shape in points.
     */
    QPointF position() const;

    /**
     * @brief setPosition set the position of the shape.
     * @param point the new position in points
     */
    void setPosition(QPointF point);

    /**
     * @brief toSvg
     * convert the shape to svg, will not include style definitions.
     * @return the svg in a string.
     */
    QString toSvg();

private:
    friend class GroupShape;
    friend class VectorLayer;

    struct Private;
    Private *const d;

    KoShape *shape();
};

#endif // LIBKIS_SHAPE_H
