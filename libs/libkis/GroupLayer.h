/*
 *  SPDX-FileCopyrightText: 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
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

