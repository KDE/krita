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
#ifndef LIBKIS_FILTERLAYER_H
#define LIBKIS_FILTERLAYER_H

#include <QObject>
#include "Node.h"
#include <Filter.h>
#include <Selection.h>

#include <kis_types.h>

#include "kritalibkis_export.h"
#include "libkis.h"
/**
 * @brief The FilterLayer class
 * A filter layer will, when compositing, take the composited
 * image up to the point of the loction of the filter layer
 * in the stack, create a copy and apply a filter.
 *
 * This means you can use blending modes on the filter layers,
 * which will be used to blend the filtered image with the original.
 *
 * Similarly, you can activate things like alpha inheritance, or
 * you can set grayscale pixeldata on the filter layer to act as
 * a mask.
 *
 * Filter layers can be animated.
 */
class KRITALIBKIS_EXPORT FilterLayer : public Node
{
    Q_OBJECT
    Q_DISABLE_COPY(FilterLayer)

public:
    explicit FilterLayer(KisImageSP image, QString name, Filter &filter, Selection &selection, QObject *parent = 0);
    explicit FilterLayer(KisAdjustmentLayerSP layer, QObject *parent = 0);
    ~FilterLayer() override;
public Q_SLOTS:

    /**
     * @brief type Krita has several types of nodes, split in layers and masks. Group
     * layers can contain other layers, any layer can contain masks.
     *
     * @return "filterlayer"
     */
    QString type() const override;

    void setFilter(Filter &filter);

    Filter * filter();
};

#endif // LIBKIS_FILTERLAYER_H

