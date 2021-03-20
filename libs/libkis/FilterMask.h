/*
 *  SPDX-FileCopyrightText: 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef LIBKIS_FILTERMASK_H
#define LIBKIS_FILTERMASK_H

#include <QObject>
#include "Node.h"
#include "Filter.h"

#include <kis_types.h>

#include "kritalibkis_export.h"
#include "libkis.h"

/**
 * @brief The FilterMask class
 * A filter mask, unlike a filter layer, will add a non-destructive filter
 * to the composited image of the node it is attached to.
 *
 * You can set grayscale pixeldata on the filter mask to adjust where the filter is applied.
 *
 * Filtermasks can be animated.
 */

class KRITALIBKIS_EXPORT FilterMask : public Node
{
    Q_OBJECT
    Q_DISABLE_COPY(FilterMask)

public:
    explicit FilterMask(KisImageSP image, QString name, Filter &filter, QObject *parent = 0);
    explicit FilterMask(KisImageSP image, KisFilterMaskSP mask, QObject *parent=0);
    ~FilterMask() override;
public Q_SLOTS:

    /**
     * @brief type Krita has several types of nodes, split in layers and masks. Group
     * layers can contain other layers, any layer can contain masks.
     *
     * @return The type of the node. Valid types are:
     * <ul>
     *  <li>paintlayer
     *  <li>grouplayer
     *  <li>filelayer
     *  <li>filterlayer
     *  <li>filllayer
     *  <li>clonelayer
     *  <li>vectorlayer
     *  <li>transparencymask
     *  <li>filtermask
     *  <li>transformmask
     *  <li>selectionmask
     *  <li>colorizemask
     * </ul>
     *
     * If the Node object isn't wrapping a valid Krita layer or mask object, and
     * empty string is returned.
     */
    QString type() const override;

    void setFilter(Filter &filter);
    Filter *filter();
};

#endif // LIBKIS_FILTERMASK_H


