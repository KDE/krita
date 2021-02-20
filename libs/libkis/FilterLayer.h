/*
 *  SPDX-FileCopyrightText: 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
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

