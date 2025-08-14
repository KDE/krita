/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef GLYPHPALETTEDIALOG_H
#define GLYPHPALETTEDIALOG_H

#include <KoDialog.h>
#include <KisQQuickWidget.h>

#include <KoSvgTextProperties.h>
#include <KoFontGlyphModel.h>
#include <KoSvgTextShape.h>
#include <QStandardItemModel>

#include "GlyphPaletteProxyModel.h"
#include "GlyphPaletteAltPopup.h"

/**
 * @brief The GlyphPaletteDialog class
 *
 * This dialog holds the glyph palette/character map for a given
 * font. It can show both the alternate glyphs for the current text,
 * and show a character map with selectable glyph alternates.
 */
class GlyphPaletteDialog: public KoDialog
{
    Q_OBJECT
public:
    GlyphPaletteDialog(QWidget *parent = nullptr);
    ~GlyphPaletteDialog();

    /**
     * @brief setGlyphModelFromProperties
     * This updates the glyph model.
     * @param properties -- text properties, first is non-inherited, second is inherited.
     * @param text -- the current text to display alternate glyphs for.
     */
    void setGlyphModelFromProperties(const QPair<KoSvgTextProperties, KoSvgTextProperties> &properties, const QString &text);

Q_SIGNALS:
    void signalInsertRichText(KoSvgTextShape *text, bool replace);
public Q_SLOTS:

    void slotInsertRichText(const int charRow, const int glyphRow = -1, const bool replace = false, const bool useCharMap = false);
    // Show the glyphalts for charRow in a pop-up located at x, y. Make sure to adjust cell-size.
    void slotShowPopupPalette(const int charRow, const int x = 0, const int y = 0, const int cellWidth = 100, const int cellHeight = 100);
    void slotHidePopupPalette();

private:
    KisQQuickWidget *m_quickWidget {0};
    GlyphPaletteAltPopup *m_altPopup {0};
    QPair<KoSvgTextProperties, KoSvgTextProperties> m_lastUsedProperties;
    KoFontGlyphModel *m_model;
    GlyphPaletteProxyModel *m_charMapModel;
};

#endif // GLYPHPALETTEDIALOG_H
