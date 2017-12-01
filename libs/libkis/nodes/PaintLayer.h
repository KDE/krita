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
#ifndef LIBKIS_PAINTLAYER_H
#define LIBKIS_PAINTLAYER_H

#include <QObject>
#include "Node.h"

#include <kis_types.h>

#include "kritalibkis_export.h"
#include "libkis.h"

/**
 * @brief The PaintLayer class
 * A paintlayer in Krita is a regular layer that shows raster or pixel graphics.
 * These can be manipulated with the paint tools.
 *
 * Paint layers can be animated.
 */

class KRITALIBKIS_EXPORT PaintLayer : public Node
{
    Q_OBJECT
    Q_DISABLE_COPY(PaintLayer)

public:
    explicit PaintLayer(KisImageSP image, QString name, QObject *parent = 0);
    explicit PaintLayer(KisPaintLayerSP layer, QObject *parent = 0);
    ~PaintLayer() override;
public Q_SLOTS:
    /**
     * @brief type Krita has several types of nodes, split in layers and masks. Group
     * layers can contain other layers, any layer can contain masks.
     *
     * @return paintlayer
     */
    virtual QString type() const override;
private:
    friend class Document;
};

#endif // LIBKIS_PAINTLAYER_H
