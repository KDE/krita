/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef GLYPHPALETTEALTPOPUP_H
#define GLYPHPALETTEALTPOPUP_H

#include <QFrame>
#include <KisQQuickWidget.h>
#include <QAbstractItemModel>

/**
 * @brief The GlyphPaletteAltPopup class
 *
 * Because QtQuickWidgets don't allow pop-ups to be drawn outside of their geometry,
 * we use this frame as a workaround, to allow the glyph-alts in the character map
 * to overlap the rest of the window.
 */
class GlyphPaletteAltPopup : public QFrame
{
    Q_OBJECT
public:
    GlyphPaletteAltPopup(QWidget *parent = nullptr);
    ~GlyphPaletteAltPopup();

    /**
     * @brief setRootIndex
     * Set the charmap root index to show glyph alts for.
     */
    void setRootIndex(const int index);
    /**
     * @brief setCellSize
     * Set the size of the pop-up relative to its cellsize.
     * The popup is 3 cells wide and 4 high, allowing for 12
     * alternates to be visible at once.
     */
    void setCellSize(const int width, const int height);
    /**
     * @brief setModel
     * Set the glyph proxy model.
     */
    void setModel(QAbstractItemModel *model);

    /**
     * @brief setMarkup
     * Set the css font markup to be used inside the palette.
     */
    void setMarkup(const QStringList &families, const int size, const int weight, const int width, const QFont::Style style, const QVariantMap &axes, const QString &language);

public Q_SLOTS:

    void slotInsertRichText(const int charRow, const int glyphRow = -1, const bool replace = false, const bool useCharMap = false);
Q_SIGNALS:
    void sigInsertRichText(int charRow, int glyphRow, bool replace, bool useCharMap);
private:
    KisQQuickWidget *m_quickWidget {0};
    QAbstractItemModel *m_charMapModel {0};
};

#endif // GLYPHPALETTEALTPOPUP_H
