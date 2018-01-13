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

