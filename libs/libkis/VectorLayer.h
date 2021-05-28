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

};

#endif // LIBKIS_VECTORLAYER_H

