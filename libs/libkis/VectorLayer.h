/*
 *  SPDX-FileCopyrightText: 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef LIBKIS_VECTORLAYER_H
#define LIBKIS_VECTORLAYER_H

#include <QObject>

#include <kis_types.h>

#include "kritalibkis_export.h"
#include "libkis.h"

#include <KoShapeControllerBase.h>

#include "Node.h"
#include "Shape.h"

/**
 * @brief The VectorLayer class
 * A vector layer is a special layer that stores
 * and shows vector shapes.
 *
 * Vector shapes all have their coordinates in points, which
 * is a unit that represents 1/72th of an inch. Keep this in
 * mind wen parsing the bounding box and position data.
 */

class KRITALIBKIS_EXPORT VectorLayer : public Node
{
    Q_OBJECT
    Q_DISABLE_COPY(VectorLayer)

public:
    explicit VectorLayer(KoShapeControllerBase* shapeController, KisImageSP image, QString name, QObject *parent = 0);
    explicit VectorLayer(KisShapeLayerSP layer, QObject *parent = 0);
    ~VectorLayer() override;
public Q_SLOTS:

    /**
     * @brief type Krita has several types of nodes, split in layers and masks. Group
     * layers can contain other layers, any layer can contain masks.
     *
     * @return vectorlayer
     */
    virtual QString type() const override;

    /**
     * @brief shapes
     * @return the list of top-level shapes in this vector layer.
     */
    QList<Shape *> shapes() const;

    /**
     * @brief toSvg
     * convert the shapes in the layer to svg.
     * @return the svg in a string.
     */
    QString toSvg();

    /**
     * @brief addShapesFromSvg
     * add shapes to the layer from a valid svg.
     * @param svg valid svg string.
     * @return the list of shapes added to the layer from the svg.
     */
    QList<Shape *> addShapesFromSvg(const QString &svg);

    /**
     * @brief shapeAtPoint
     * check if the position is located within any non-group shape's boundingBox() on the current layer.
     * @param position a QPointF of the position.
     * @return the shape at the position, or None if no shape is found.
     */
    Shape* shapeAtPosition(const QPointF &position) const;

    /**
     * @brief shapeInRect
     * get all non-group shapes that the shape's boundingBox() intersects or is contained within a given rectangle on the current layer.
     * @param rect a QRectF
     * @param omitHiddenShapes true if non-visible() shapes should be omitted, false if they should be included. \p omitHiddenShapes defaults to true.
     * @param containedMode false if only shapes that are within or intersect with the outline should be included, true if only shapes that are fully contained within the outline should be included. \p containedMode defaults to false
     * @return returns a list of shapes.
     */
    QList<Shape *> shapesInRect(const QRectF &rect, bool omitHiddenShapes = true, bool containedMode = false) const;

    /**
     * @brief createGroupShape
     * combine a list of top level shapes into a group.
     * @param name the name of the shape.
     * @param shapes list of top level shapes.
     * @return if successful, a GroupShape object will be returned.
     */
    Shape* createGroupShape(const QString &name, QList<Shape *> shapes) const;

};

#endif // LIBKIS_VECTORLAYER_H

