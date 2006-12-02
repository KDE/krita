/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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

#ifndef KOTEXTSELECTIONHANDLER_H
#define KOTEXTSELECTIONHANDLER_H

#include <KoToolSelection.h>

#include <koffice_export.h>

#include <QColor>

class KoTextShape;
class KoTextShapeData;
class QTextCursor;

/**
 * The public class that is able to manipulate selected text.
 */
class KOTEXT_EXPORT KoTextSelectionHandler : public KoToolSelection {
public:
    KoTextSelectionHandler() {}

    /**
     * Make the currently selected text bold
     * @param bold if true, make text have weight bold, if false make normal.
     */
    void bold(bool bold);

    /**
     * Make the currently selected text italic
     * @param italic if true, make italic, if false make slant normal.
     */
    void italic(bool italic);

    /**
     * Make the currently selected text underlined.
     * @param underline if true make the text have a continues underline.
     */
    void underline(bool underline);

    /**
     * Make the currently selected text have a line through it.
     * @param strikeout if true, make the text have a strike out through it.
     */
    void strikeOut(bool strikeout);

    /**
     * Insert a frame break at the cursor position, moving the rest of the text to the next frame.
     */
    void insertFrameBreak();

    /**
     * Alter the selection to have param font size.
     * @param size the new font size in points.
     */
    void setFontSize(int size);

    /**
     * Alter the selections font size to be slightly bigger.
     */
    void increaseFontSize();

    /**
     * Alter the selections font size to be slightly smaller.
     */
    void decreaseFontSize();
    void setHorizontalTextAlignment(Qt::Alignment align);
    void setVerticalTextAlignment(Qt::Alignment align);
    void setTextColor(const QColor &color);
    void setTextBackgroundColor(const QColor &color);
    void insert(const QString &text);
    QString selectedText() const;
    void selectFont(QWidget *parent = 0);

protected:
    friend class KoTextTool;
    void setShape(KoTextShape *shape) { m_textShape = shape; }
    void setShapeData(KoTextShapeData *data) { m_textShapeData = data; }
    void setCaret(QTextCursor *caret) { m_caret = caret; }

private:
    KoTextShape *m_textShape;
    KoTextShapeData *m_textShapeData;
    QTextCursor *m_caret;
};

#endif
