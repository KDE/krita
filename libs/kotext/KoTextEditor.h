/* This file is part of the KDE project
* Copyright (C) 2009 Pierre Stirnweiss <pstirnweiss@googlemail.com>
* Copyright (C) 2009 Thomas Zander <zander@kde.org>
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

#ifndef KOTEXTEDITOR_H
#define KOTEXTEDITOR_H

#include "kotext_export.h"

#include <KoGenChange.h>
#include "KoText.h"
#include <KoToolSelection.h>

#include <QClipboard>
#include <QMetaType>
#include <QTextCursor>

class KoCharacterStyle;
class KoInlineObject;
class KoParagraphStyle;

class QTextBlock;
class QTextCharFormat;
class QTextBlockFormat;
class QTextDocument;
class QTextDocumentFragment;
class QString;
class QUndoCommand;

class KOTEXT_EXPORT KoTextEditor: public KoToolSelection
{
    Q_OBJECT
public:
    KoTextEditor(QTextDocument *document);

    virtual ~KoTextEditor();

/*    QTextCursor ()
QTextCursor ( QTextDocument * document )
QTextCursor ( QTextFrame * frame )
QTextCursor ( const QTextBlock & block )
QTextCursor ( const QTextCursor & cursor )
~QTextCursor ()
*/
    void updateDefaultTextDirection(KoText::Direction direction);

    bool operator!=(const QTextCursor &other) const;

    bool operator<(const QTextCursor &other) const;

    bool operator<=(const QTextCursor &other) const;

    bool operator==(const QTextCursor &other) const;

    bool operator>(const QTextCursor &other) const;

    bool operator>=(const QTextCursor &other) const;

public slots:
    ///This should be used only as read-only cursor or within a QUndoCommand sub-class which will be added to the textEditor with addCommand. For examples of proper implementation of such undoCommands, see the TextShape commands.
    QTextCursor* cursor();

    void addCommand(QUndoCommand *command);

    void registerTrackedChange(QTextCursor &selection, KoGenChange::Type changeType, QString title, QTextFormat &format, QTextFormat &prevFormat, bool applyToWholeBlock = false);

    void bold(bool bold);

    void italic(bool italic);

    void underline(bool underline);

    void strikeOut(bool strikeOut);

    void setHorizontalTextAlignment(Qt::Alignment align);

    void setVerticalTextAlignment(Qt::Alignment align);

    void increaseIndent();

    void decreaseIndent();

    void increaseFontSize();

    void decreaseFontSize();

    void setFontFamily(const QString &font);

    void setFontSize(int size);

    void setTextColor(const QColor &color);

    void setTextBackgroundColor(const QColor &color);

    void setDefaultFormat();

    void setStyle(KoParagraphStyle *style);

    void setStyle(KoCharacterStyle *style);

    /**
    * Insert an inlineObject (such as a variable) at the current cursor position. Possibly replacing the selection.
    * @param inliner the object to insert.
    */
    void insertInlineObject(KoInlineObject *inliner);

    /**
    * At the current cursor position, insert a marker that marks the next word as being part of the index.
    * @returns returns true when successful, or false if failed.  Failure can be because there is no word
    *  at the cursor position or there already is an index marker available.
    */
    bool insertIndexMarker();

    /// add a bookmark on current cursor location or current selection
    void addBookmark(const QString &name);

    /**
    * Insert a frame break at the cursor position, moving the rest of the text to the next frame.
    */
    void insertFrameBreak();

    /// delete all inline objects in current cursor position or selection
    bool deleteInlineObjects(bool backward = false);


/// QTextCursor methods
    int anchor() const;

    bool atBlockEnd() const;

    bool atBlockStart() const;

    bool atEnd() const;

    bool atStart() const;

    void beginEditBlock();

    QTextBlock block() const;

    QTextCharFormat blockCharFormat() const;

    QTextBlockFormat blockFormat() const;

    int blockNumber() const;

    QTextCharFormat charFormat() const;

    void clearSelection();

    int columnNumber() const;

    void deleteChar();

    void deletePreviousChar();

    QTextDocument *document() const;

    void endEditBlock();

    bool hasComplexSelection() const;

    bool hasSelection();

    void insertBlock();

    void insertBlock(const QTextBlockFormat &format);

    void insertBlock(const QTextBlockFormat &format, const QTextCharFormat &charFormat);

    void insertFragment(const QTextDocumentFragment &fragment);

     /**
     * Insert a table at the current cursor position.
     * @param rows the number of rows in the created table.
     * @param columns the number of columns in the created table.
     */
    void insertTable(int rows, int columns);

    void insertText(const QString &text);

    void insertText(const QString &text, const QTextCharFormat &format);

//    void joinPreviousEditBlock ();

    void mergeBlockCharFormat( const QTextCharFormat &modifier);

    void mergeBlockFormat( const QTextBlockFormat &modifier);

    void mergeCharFormat(const QTextCharFormat &modifier);

    bool movePosition(QTextCursor::MoveOperation operation, QTextCursor::MoveMode mode = QTextCursor::MoveAnchor, int n = 1);

    void newLine();

    int position() const;

    void removeSelectedText();

    void select(QTextCursor::SelectionType selection);

    QString selectedText() const;

    QTextDocumentFragment selection() const;

    int selectionEnd() const;

    int selectionStart() const;

    void setBlockCharFormat(const QTextCharFormat &format);

    void setBlockFormat(const QTextBlockFormat &format);

    void setCharFormat(const QTextCharFormat &format);

    void setPosition(int pos, QTextCursor::MoveMode mode = QTextCursor::MoveAnchor);

    void setVisualNavigation(bool on);

    bool visualNavigation() const;

    bool isBidiDocument() const;

signals:
    void isBidiUpdated();

private:
    Q_PRIVATE_SLOT(d, void documentCommandAdded())
    Q_PRIVATE_SLOT(d, void runDirectionUpdater())

    class Private;
    Private* const d;
};

Q_DECLARE_METATYPE(KoTextEditor*)

#endif // KOTEXTEDITOR_H
