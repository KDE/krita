/*
 *  SPDX-FileCopyrightText: 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef LIBKIS_TRANSPARENCYMASK_H
#define LIBKIS_TRANSPARENCYMASK_H

#include <QObject>
#include "Node.h"

#include <kis_types.h>

#include "kritalibkis_export.h"
#include "libkis.h"

/**
 * @brief The TransparencyMask class
 * A transparency mask is a mask type node that can be used
 * to show and hide parts of a layer.
 *
 */
class KRITALIBKIS_EXPORT TransparencyMask : public Node
{
    Q_OBJECT
    Q_DISABLE_COPY(TransparencyMask)

public:
    explicit TransparencyMask(KisImageSP image, QString name, QObject *parent = 0);
    explicit TransparencyMask(KisImageSP image, KisTransparencyMaskSP mask, QObject *parent = 0);
    ~TransparencyMask() override;
public Q_SLOTS:

    /**
     * @brief type Krita has several types of nodes, split in layers and masks. Group
     * layers can contain other layers, any layer can contain masks.
     *
     * @return transparencymask
     *
     * If the Node object isn't wrapping a valid Krita layer or mask object, and
     * empty string is returned.
     */
    virtual QString type() const override;

    Selection *selection() const;

    void setSelection(Selection *selection);
};

#endif // LIBKIS_TRANSPARENCYMASK_H

