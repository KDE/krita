/*
 * SPDX-FileCopyrightText: 2014 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef DLG_LAYERSPLIT
#define DLG_LAYERSPLIT

#include <KoDialog.h>
#include <KoColorSet.h>
#include <KisPaletteChooser.h>
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
    KoColorSetSP palette() const;

private Q_SLOTS:

    void slotApplyClicked();
    void slotSetPalette(KoColorSetSP pal);
    void slotChangeMode(int);

private:

    friend class LayerSplit;
    bool m_modeToMask;

    WdgLayerSplit *m_page {0};
    KisPaletteChooser *m_colorSetChooser {0};
    KoColorSetSP m_palette {0};
};

#endif // DLG_LAYERSPLIT
