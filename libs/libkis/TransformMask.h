/*
 *  SPDX-FileCopyrightText: 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef LIBKIS_TRANSFORMMASK_H
#define LIBKIS_TRANSFORMMASK_H

#include <QObject>
#include "Node.h"

#include <kis_types.h>

#include "kritalibkis_export.h"
#include "libkis.h"

/**
 * @brief The TransformMask class
 * A transform mask is a mask type node that can be used
 * to store transformations.
 */
class KRITALIBKIS_EXPORT TransformMask : public Node
{
    Q_OBJECT
    Q_DISABLE_COPY(TransformMask)

public:
    explicit TransformMask(KisImageSP image, QString name, QObject *parent = 0);
    explicit TransformMask(KisImageSP image, KisTransformMaskSP mask, QObject *parent = 0);
    ~TransformMask() override;
public Q_SLOTS:

    /**
     * @brief type Krita has several types of nodes, split in layers and masks. Group
     * layers can contain other layers, any layer can contain masks.
     *
     * @return transformmask
     *
     * If the Node object isn't wrapping a valid Krita layer or mask object, and
     * empty string is returned.
     */
    virtual QString type() const override;

    QTransform finalAffineTransform() const;

};

#endif // LIBKIS_TRANSFORMMASK_H

