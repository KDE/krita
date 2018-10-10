/*
 * Copyright (C) 2014 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef DLG_LAYERSPLIT
#define DLG_LAYERSPLIT

#include <KoDialog.h>
#include <KoColorSet.h>
#include <KisPaletteListWidget.h>
#include <kis_types.h>

#include "wdg_layersplit.h"

/**
 * This dialog allows the user to create a selection mask based
 * on a (range of) colors.
 */
class DlgLayerSplit: public KoDialog
{

    Q_OBJECT

public:

    DlgLayerSplit();
    ~DlgLayerSplit() override;

    bool createBaseGroup() const;
    bool createSeparateGroups() const;
    bool lockAlpha() const;
    bool hideOriginal() const;
    bool sortLayers() const;
    bool disregardOpacity() const;
    int fuzziness() const;
    KoColorSet* palette() const;

private Q_SLOTS:

    void applyClicked();
    void slotSetPalette(KoColorSet *pal);

private:
    WdgLayerSplit *m_page {0};
    KisPaletteListWidget *m_colorSetChooser {0};
    KoColorSet *m_palette {0};
};

#endif // DLG_LAYERSPLIT
