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

};

#endif // LIBKIS_VECTORLAYER_H

