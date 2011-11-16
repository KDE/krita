/* This file is part of the KDE project
* Copyright (C) 2009 Pierre Stirnweiss <pstirnweiss@googlemail.com>
* Copyright (C) 2009 Thomas Zander <zander@kde.org>
* Copyright (C) 2011 Boudewijn Rempt <boud@valdyas.org>
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
#include "styles/KoListStyle.h"
#include <KoToolSelection.h>

#include <QClipboard>
#include <QMetaType>
#include <QTextCursor>
#include <QTextFrame>

class KoDocumentRdfBase;
class KoCharacterStyle;
class KoInlineObject;
class KoParagraphStyle;
class KoInlineNote;
class KoInlineCite;
class KoBibliographyInfo;
class KoCanvasBase;
class KoTableOfContentsGeneratorInfo;
class KoShapeController;
class KoTextAnchor;

class QTextBlock;
class QTextCharFormat;
class QTextBlockFormat;
class QTextDocument;
class QTextDocumentFragment;
class QString;
class KUndo2Command;

/**
 * KoTextEditor is a wrapper around QTextCursor. It handles undo/redo and change
 * tracking for all editing commands.
 */
class KOTEXT_EXPORT KoTextEditor: public QObject
{
    Q_OBJECT
public:

    enum MoveOperation {
        PreviousChar,
        NextChar
    };

    enum ChangeListFlag {
        NoFlags = 0,
        ModifyExistingList = 1,
        MergeWithAdjacentList = 2,
        MergeExactly = 4,
        CreateNumberedParagraph = 8,
        AutoListStyle = 16,
        DontUnsetIfSame = 32 /// do not unset the current list style if it is already been set the same
    };
    Q_DECLARE_FLAGS(ChangeListFlags, ChangeListFlag)

    KoTextEditor(QTextDocument *document);

    virtual ~KoTextEditor();

    /**
     * Retrieves the texteditor for the document of the first text shape in the current
     * set of selected shapes on the given canvas.
     *
     * @param canvas the canvas we will check for a suitable selected shape.
     * @returns a texteditor, or 0 if there is no shape active that has a QTextDocument as
     * userdata
     */
    static KoTextEditor *getTextEditorFromCanvas(KoCanvasBase *canvas);


public: // KoToolSelection overloads

    /// returns true if the wrapped QTextCursor has a selection.
    bool hasSelection() const;

    /** returns true if the current cursor position is protected from editing
     * @param cached use cached value if available.
     */
    bool isEditProtected(bool useCached = false) const;

public:

    /// Called when loading is done to check whether there's bidi text in the document.
    void finishedLoading();

    void updateDefaultTextDirection(KoText::Direction direction);

    bool operator!=(const QTextCursor &other) const;

    bool operator<(const QTextCursor &other) const;

    bool operator<=(const QTextCursor &other) const;

    bool operator==(const QTextCursor &other) const;

    bool operator>(const QTextCursor &other) const;

    bool operator>=(const QTextCursor &other) const;

private:

    // for the call to KoTextLoader::loadBody, which has a QTextCursor
    friend class KoTextPaste;

    // from KoTextEditor_p.h
    friend class CharFormatVisitor;

    // our commands can have access to us
    friend class DeleteTableRowCommand;
    friend class DeleteTableColumnCommand;
    friend class InsertTableRowCommand;
    friend class InsertTableColumnCommand;
    friend class ChangeTrackedDeleteCommand;
    friend class DeleteCommand;

    // for unittests
    friend class TestKoInlineTextObjectManager;

    // temporary...
    friend class TextShape;
    friend class TextTool;

    /**
     * This should be used only as read-only cursor or within a KUndo2Command sub-class which
     * will be added to the textEditor with addCommand. For examples of proper implementation of
     * such undoCommands, see the TextShape commands.
     */
    QTextCursor* cursor();

public slots:

    // XXX: make this private as  well
    void addCommand(KUndo2Command *command, bool addCommandToStack = true);

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

    void setFontSize(qreal size);

    void setTextColor(const QColor &color);

    void setTextBackgroundColor(const QColor &color);

    void setDefaultFormat();

    void setStyle(KoParagraphStyle *style);

    void setStyle(KoCharacterStyle *style);

    /**
     * Insert an inlineObject (such as a variable) at the current cursor position. Possibly replacing the selection.
     * @param inliner the object to insert.
     * @param cmd a parent command for the commands created by this methods. If present, the commands
     *    will not be added to the document's undo stack automatically.
     */
    void insertInlineObject(KoInlineObject *inliner, KUndo2Command *parent = 0);

    /**
     * update the position of all inline objects from the given start point to the given end point.
     * @param start start position for updating. If 0, we update from the start of the document
     * @param end end position for updating. If -1, we update to the end of the document
     */
    void updateInlineObjectPosition(int start = 0, int end = -1);

    /**
    * At the current cursor position, insert a marker that marks the next word as being part of the index.
    * @returns returns the index marker when successful, or 0 if failed.  Failure can be because there is no word
    *  at the cursor position or there already is an index marker available.
    */
    KoInlineObject *insertIndexMarker();

    /// add a bookmark on current cursor location or current selection
    void addBookmark(const QString &name);

    /**
     * Insert a frame break at the cursor position, moving the rest of the text to the next frame.
     */
    void insertFrameBreak();

    /// delete all inline objects in current cursor position or selection
    bool deleteInlineObjects(bool backward = false);

    /**
     * paste the given mimedata object at the current position
     * @param mimeData: the mimedata containing text, html or odf
     * @param shapeController the canvas' shapeController
     * @param pasteAsText: if true, paste without formatting
     */
    void paste(const QMimeData *mimeData,
               KoShapeController *shapeController,
               bool pasteAsText=false);

    /**
     * Insert the selection from the given KoTextEditor. If there is no selection, the entire
     * content of the document behind the editor is used. This changes the cursor position of
     * the editor instance. Note that this is another text editor, preferably on another document!
     *
     * @param editor the KoTextEditor instance.
     * @param shapeController the canvas' shapeController
     * @param pasteAsText: if true, paste without formatting
     * @returns true if the operation succeeded
     */
    bool paste(KoTextEditor *editor,
               KoShapeController *shapeController,
               bool pasteAsText = false);

    /**
     * Delete one character in the specified direction.
     * @param direction the direction into which we delete. Valid values are
     * @param trackChanges if true, track this deletion in the changetracker
     * @param shapeController the canvas' shapeController
     */
    void deleteChar(MoveOperation direction, bool trackChanges,
                    KoShapeController *shapeController);

    /**
     * @param numberingEnabled when true, we will enable numbering for the current paragraph (block).
     */
    void toggleListNumbering(bool numberingEnabled);

    /**
     * change the current block's list properties
     */
    void setListProperties(KoListStyle::Style style,
                           int level = 0,
                           ChangeListFlags flags = ChangeListFlags(ModifyExistingList | MergeWithAdjacentList));

    // -------------------------------------------------------------
    // Wrapped QTextCursor methods
    // -------------------------------------------------------------

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

    const QTextDocument *document() const;

    void endEditBlock();

    bool hasComplexSelection() const;

    void insertBlock();

    void insertBlock(const QTextBlockFormat &format);

    void insertBlock(const QTextBlockFormat &format, const QTextCharFormat &charFormat);

// NOT part of the api, since QTextDocumentFragment translates to html, losing all formatting.
// so intentionally not exposed.
//    void insertFragment(const QTextDocumentFragment &fragment);

     /**
     * Insert a table at the current cursor position.
     * @param rows the number of rows in the created table.
     * @param columns the number of columns in the created table.
     */
    void insertTable(int rows, int columns);

     /**
     * Insert a table row above the current cursor position (if in a table).
     */
    void insertTableRowAbove();

     /**
     * Insert a table row below the current cursor position (if in a table).
     */
    void insertTableRowBelow();

     /**
     * Insert a table column to the left of the current cursor position (if in a table).
     */
    void insertTableColumnLeft();

     /**
     * Insert a table column to the right of the current cursor position (if in a table).
     */
    void insertTableColumnRight();

     /**
     * Delete a table column where the cursor is (if in a table).
     */
    void deleteTableColumn();

     /**
     * Delete a table row where the cursor is (if in a table).
     */
    void deleteTableRow();

     /**
     * Merge table cells (selected by the cursor).
     */
    void mergeTableCells();

     /**
     * Split table cells (selected by the cursor) that were previously merged.
     */
    void splitTableCells();

    /**
     * Insert a footnote at the current cursor position
     * @return a pointer to the inserted footnote
     */
    KoInlineNote *insertFootNote();

    /**
     * Insert an endnote at the current cursor position
     * @return a pointer to the inserted endnote
     */
    KoInlineNote *insertEndNote();

    /**
     * Insert a table of Contents at the current cursor position.
     */
    void insertTableOfContents(KoTableOfContentsGeneratorInfo *info);

    /**
     * Configures various values of a ToC to the one passed in info
     */
    void setTableOfContentsConfig(KoTableOfContentsGeneratorInfo *info, QTextBlock block);

    void insertBibliography();

    KoInlineCite *insertCitation();

    void insertText(const QString &text);

    void insertText(const QString &text, const QTextCharFormat &format);

    void insertHtml(const QString &html);
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

// intentionally commented out: these  are unimplemented.

//    void setBlockCharFormat(const QTextCharFormat &format);

   void setBlockFormat(const QTextBlockFormat &format);

   void setCharFormat(const QTextCharFormat &format);

//    void setTableFormat(const QTextTableFormat &format);

    void setPosition(int pos, QTextCursor::MoveMode mode = QTextCursor::MoveAnchor);

    void setVisualNavigation(bool on);

    bool visualNavigation() const;

    bool isBidiDocument() const;

    const QTextFrame *currentFrame () const;
    const QTextList *currentList () const;
    const QTextTable *currentTable () const;

signals:
    void isBidiUpdated();
    void cursorPositionChanged();

protected:
    bool recursiveProtectionCheck(QTextFrame::iterator it) const;

private:
    Q_PRIVATE_SLOT(d, void documentCommandAdded())
    Q_PRIVATE_SLOT(d, void runDirectionUpdater())

    class Private;
    Private* const d;
};

Q_DECLARE_METATYPE(KoTextEditor*)
Q_DECLARE_METATYPE(bool *)
Q_DECLARE_OPERATORS_FOR_FLAGS(KoTextEditor::ChangeListFlags)

#endif // KOTEXTEDITOR_H
