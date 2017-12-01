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
#ifndef LIBKIS_GROUPLAYER_H
#define LIBKIS_GROUPLAYER_H

#include <QObject>
#include "Node.h"

#include <kis_types.h>

#include "kritalibkis_export.h"
#include "libkis.h"

/**
 * @brief The GroupLayer class
 * A group layer is a layer that can contain other layers.
 * In Krita, layers within a group layer are composited
 * first before they are added into the composition code for where
 * the group is in the stack. This has a significant effect on how
 * it is interpreted for blending modes.
 *
 * PassThrough changes this behaviour.
 *
 * Group layer cannot be animated, but can contain animated layers or masks.
 */
class KRITALIBKIS_EXPORT GroupLayer : public Node
{
    Q_OBJECT
    Q_DISABLE_COPY(GroupLayer)

public:
    explicit GroupLayer(KisImageSP image, QString name, QObject *parent = 0);
    explicit GroupLayer(KisGroupLayerSP layer, QObject *parent = 0);
    ~GroupLayer() override;
public Q_SLOTS:

    /**
     * @brief type Krita has several types of nodes, split in layers and masks. Group
     * layers can contain other layers, any layer can contain masks.
     *
     * @return grouplayer
     */
    virtual QString type() const override;

    /**
     * @brief setPassThroughMode
     * This changes the way how compositing works.
     * Instead of compositing all the layers before compositing it with the rest of the image,
     * the group layer becomes a sort of formal way to organise everything.
     *
     * Passthrough mode is the same as it is in photoshop,
     * and the inverse of SVG's isolation attribute(with passthrough=false being the same as
     * isolation="isolate").
     *
     * @param passthrough whether or not to set the layer to passthrough.
     */
    void setPassThroughMode(bool passthrough);

    /**
     * @brief passThroughMode
     * @return returns whether or not this layer is in passthrough mode. @see setPassThroughMode
     */
    bool passThroughMode() const;
};

#endif // LIBKIS_GROUPLAYER_H

