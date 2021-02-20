/*
 *  SPDX-FileCopyrightText: 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef LIBKIS_SELECTIONMASK_H
#define LIBKIS_SELECTIONMASK_H

#include <QObject>
#include "Node.h"

#include <kis_types.h>

#include "kritalibkis_export.h"
#include "libkis.h"

/**
 * @brief The SelectionMask class
 * A selection mask is a mask type node that can be used
 * to store selections. In the gui, these are referred to
 * as local selections.
 *
 * A selection mask can hold both raster and vector selections, though
 * the API only supports raster selections.
 */
class KRITALIBKIS_EXPORT SelectionMask : public Node
{
    Q_OBJECT
    Q_DISABLE_COPY(SelectionMask)

public:
    explicit SelectionMask(KisImageSP image, QString name, QObject *parent = 0);
    explicit SelectionMask(KisImageSP image, KisSelectionMaskSP mask, QObject *parent = 0);
    ~SelectionMask() override;
public Q_SLOTS:

    /**
     * @brief type Krita has several types of nodes, split in layers and masks. Group
     * layers can contain other layers, any layer can contain masks.
     *
     * @return selectionmask
     *
     * If the Node object isn't wrapping a valid Krita layer or mask object, and
     * empty string is returned.
     */
    virtual QString type() const override;

    Selection *selection() const;

    void setSelection(Selection *selection);
};

#endif // LIBKIS_SELECTIONMASK_H

