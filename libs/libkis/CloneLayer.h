/*
 *  SPDX-FileCopyrightText: 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef LIBKIS_CLONELAYER_H
#define LIBKIS_CLONELAYER_H

#include <QObject>
#include "Node.h"

#include <kis_types.h>

#include "kritalibkis_export.h"
#include "libkis.h"

/**
 * @brief The CloneLayer class
 * A clone layer is a layer that takes a reference inside the image
 * and shows the exact same pixeldata.
 *
 * If the original is updated, the clone layer will update too.
 */

class KRITALIBKIS_EXPORT CloneLayer : public Node
{
    Q_OBJECT
    Q_DISABLE_COPY(CloneLayer)

public:
    explicit CloneLayer(KisImageSP image, QString name, KisLayerSP source, QObject *parent = 0);

    /**
     * @brief CloneLayer
     * function for wrapping a preexisting node into a clonelayer object.
     * @param layer the clone layer
     * @param parent the parent QObject
     */
    explicit CloneLayer(KisCloneLayerSP layer, QObject *parent = 0);
    ~CloneLayer() override;
public Q_SLOTS:

    /**
     * @brief type Krita has several types of nodes, split in layers and masks. Group
     * layers can contain other layers, any layer can contain masks.
     *
     * @return clonelayer
     */
    virtual QString type() const override;
};

#endif // LIBKIS_PAINTLAYER_H

