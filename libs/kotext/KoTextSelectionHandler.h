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
#include <KoShape.h>

#include <kotext_export.h>

#include <QColor>

class KoParagraphStyle;
class KoCharacterStyle;
class KoTextShapeData;
class KoVariable;
class QTextCursor;
class QStackedWidget;
class KoTextLocator;

/**
 * The public class that is able to manipulate selected text.
 */
class KOTEXT_EXPORT KoTextSelectionHandler : public KoToolSelection {
    Q_OBJECT
public:
    explicit KoTextSelectionHandler(QObject *parent = 0);
    ~KoTextSelectionHandler();

    QString selectedText() const;

    /**
     * At the current cursor position, insert a marker that marks the next word as being part of the index.
     * @returns returns true when successful, or false if failed.  Failure can be because there is no word
     *  at the cursor position or there already is an index marker available.
     */
    bool insertIndexMarker();

public slots:

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
    void selectFont(QWidget *parent = 0);
    void increaseIndent();
    void decreaseIndent();
    void nextParagraph();

    /**
     * Insert a variable at the current cursor position. Possibly replacing the selection.
     * @param variable the new variable.
     */
    void insertVariable(KoVariable *variable);

    /**
     * Set the selected text to follow the layout of the paragraph style.
     * @param style the new style for all selected paragraphs, or the parag where the cursor is in now.
     */
    void setStyle(KoParagraphStyle* style);

    /**
     * Set the selected text to follow the layout of the character style.
     * @param style the new style for all selected text, or for the text that will be typed where the cursor is now.
     */
    void setStyle(KoCharacterStyle* style);

    /**
     * @return the QTextCursor caret or NULL if no caret was set so far.
     */
    QTextCursor caret() const;

protected:
    friend class TextTool;
    void setShape(KoShape *shape);
    void setShapeData(KoTextShapeData *data);
    void setCaret(QTextCursor *caret);

private:
    class Private;
    Private * const d;
};

#endif
