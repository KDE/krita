/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
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

#ifndef KOTEXTTOOL_H
#define KOTEXTTOOL_H

#include "TextShape.h"

#include <KoTextSelectionHandler.h>
#include <KoTool.h>

#include <QTextCursor>
#include <QAction>
#include <QHash>
#include <QTextBlock>

class KoAction;
class KoStyleManager;
class KoTextEditingPlugin;
class KoColor;
class KoColorSetAction;
class KoBookmarkManager;
class UndoTextCommand;
class InsertCharacter;
class ChangeTracker;

class QUndoCommand;
class KFontSizeAction;
class KFontAction;

/**
 * This is the tool for the text-shape (which is a flake-based plugin).
 */
class TextTool : public KoTool {
    Q_OBJECT
public:
    explicit TextTool(KoCanvasBase *canvas);
    ~TextTool();

    /// reimplemented from superclass
    virtual void paint( QPainter &painter, const KoViewConverter &converter );

    /// reimplemented from superclass
    virtual void mousePressEvent( KoPointerEvent *event ) ;
    /// reimplemented from superclass
    virtual void mouseDoubleClickEvent( KoPointerEvent *event );
    /// reimplemented from superclass
    virtual void mouseMoveEvent( KoPointerEvent *event );
    /// reimplemented from superclass
    virtual void mouseReleaseEvent( KoPointerEvent *event );
    /// reimplemented from superclass
    virtual void keyPressEvent(QKeyEvent *event);
    /// reimplemented from superclass
    virtual void keyReleaseEvent(QKeyEvent *event);

    /// reimplemented from superclass
    virtual void activate (bool temporary=false);
    /// reimplemented from superclass
    virtual void deactivate();
    /// reimplemented from superclass
    virtual void copy() const;
    /// reimplemented from superclass
    virtual bool paste();
    /// reimplemented from superclass
    virtual void repaintDecorations();

    /// reimplemented from superclass
    virtual KoToolSelection* selection();
    /// reimplemented from superclass
    virtual QWidget *createOptionWidget();
    /// reimplemented from superclass
    virtual QVariant inputMethodQuery(Qt::InputMethodQuery query, const KoViewConverter &converter) const;
    /// reimplemented from superclass
    virtual void inputMethodEvent(QInputMethodEvent * event);

    void startTextEditingPlugin(const QString &pluginId);

    bool isBidiDocument() const;

public slots:
    /// add a command to the undo stack, executing it as well.
    void addCommand(QUndoCommand *command);
    /// reimplemented from KoTool
    virtual void resourceChanged (int key, const QVariant &res);

signals:
    /// emitted every time a different styleManager is set.
    void styleManagerChanged(KoStyleManager *manager);
    /// emitted every time a caret move leads to a different character format being under the caret
    void charFormatChanged(const QTextCharFormat &format);
    /// emitted every time a caret move leads to a different paragraph format being under the caret
    void blockFormatChanged(const QTextBlockFormat &format);
    /// emitted every time a caret move leads to a different paragraph format being under the caret
    void blockChanged(const QTextBlock &block);

private slots:
    /// insert a non breaking space at the caret position
    void nonbreakingSpace();
    /// insert a non breaking hyphen at the caret position
    void nonbreakingHyphen();
    /// insert a soft hyphen at the caret position
    void softHyphen();
    /// insert a linebreak at the caret position
    void lineBreak();
    /// align all of the selected text left
    void alignLeft();
    /// align all of the selected text right
    void alignRight();
    /// align all of the selected text centered
    void alignCenter();
    /// align all of the selected text block-justified
    void alignBlock();
    /// make the selected text switch to be super-script
    void superScript(bool);
    /// make the selected text switch to be sub-script
    void subScript(bool);
    /// move the paragraph indent of the selected text to be less (left on LtR text)
    void decreaseIndent();
    /// move the paragraph indent of the selected text to be more (right on LtR text)
    void increaseIndent();
    /// Default Format
    void textDefaultFormat();
    /// see KoTextSelectionHandler::insertIndexMarker
    void insertIndexMarker();
    /// insert a bookmark on current text cursor location or selection
    void addBookmark();
    /// go to previously bookmarked text cursor location or selection
    void selectBookmark();
    /// shows a dialog to alter the paragraph properties
    void formatParagraph();
    /// When enabled, make the change tracker record changes made while typing
    void toggleTrackChanges(bool);
    /// select all text in the current document.
    void selectAll();
    /// show the style manager
    void showStyleManager();
    /// change color of a selected text
    void setTextColor(const KoColor &color);

    /// add a KoDocument wide undo command which will call undo on the qtextdocument.
    void addUndoCommand();

    /// slot to call when a series of commands is started that together need to become 1 undo action.
    void startMacro(const QString &title);
    /// slot to call when a series of commands has ended that together should be 1 undo action.
    void stopMacro();

    /// delete previously bookmarked text cursor location or selection (from the Select Bookmark dialog)
    void deleteBookmark(const QString &name);

    /// show the insert special character docker.
    void insertSpecialCharacter();

    /// method that will be called in an alternative thread for updating the paragraph direction at a character pos
    void updateParagraphDirection(const QVariant &variant);
    /// method that will be called in the UI thread directly after the one above
    void updateParagraphDirectionUi();

    void selectFont();
    void shapeAddedToCanvas();

private:
    void repaintCaret();
    void repaintSelection();
    void repaintSelection(int from, int to);
    void ensureCursorVisible();
    QRectF textRect(int startPosition, int endPosition) const;
    int pointToPosition(const QPointF & point) const;
    void updateSelectionHandler();
    void updateActions();
    void updateStyleManager();
    void setShapeData(KoTextShapeData *data);

    void editingPluginEvents();
    void finishedWord();
    void finishedParagraph();

    void startKeyPressMacro();

private:
    friend class UndoTextCommand;
    friend class TextCommandBase;
    friend class ChangeTracker;
    TextShape *m_textShape;
    KoTextShapeData *m_textShapeData;
    QTextCursor m_caret;
    ChangeTracker *m_changeTracker;
    KoTextSelectionHandler m_selectionHandler;
    KoBookmarkManager *m_bookmarkManager;
    bool m_allowActions;
    bool m_allowAddUndoCommand;
    bool m_trackChanges;
    bool m_allowResourceProviderUpdates;
    bool m_needSpellChecking;
    bool m_processingKeyPress; // all undo commands generated from key-presses should be combined.
    int m_prevCursorPosition; /// used by editingPluginEvents

    QAction *m_actionFormatBold;
    QAction *m_actionFormatItalic;
    QAction *m_actionFormatUnderline;
    QAction *m_actionFormatStrikeOut;
    QAction *m_actionAlignLeft;
    QAction *m_actionAlignRight;
    QAction *m_actionAlignCenter;
    QAction *m_actionAlignBlock;
    QAction *m_actionFormatSuper;
    QAction *m_actionFormatSub;
    QAction *m_actionFormatIncreaseIndent;
    QAction *m_actionFormatDecreaseIndent;
    KFontSizeAction *m_actionFormatFontSize;
    KFontAction *m_actionFormatFontFamily;
    KoColorSetAction *m_actionFormatTextColor;

    QHash<QString, KoTextEditingPlugin*> m_textEditingPlugins;
    KoTextEditingPlugin *m_spellcheckPlugin;

    QUndoCommand *m_currentCommand;
    bool m_currentCommandHasChildren;

    // update Parag direction will be multi-threaded.
    struct UpdatePageDirection {
        KoAction *action;
        QTextBlock block;
        KoText::Direction direction;
    };
    UpdatePageDirection m_updateParagDirection;

    /// structure that allows us to remember the text position and selection of previously edited documents.
    struct TextSelection {
        QTextDocument *document; // be warned that this may end up being a dangling pointer, so don't use.
        int position;
        int anchor;
    };
    QList<TextSelection> m_previousSelections;

    InsertCharacter *m_specialCharacterDocker;
};

#endif
