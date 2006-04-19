/* This file is part of the KDE project
   Copyright (C) 2001-2006 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef kotextview_h
#define kotextview_h

#include <qobject.h>
#include <qpoint.h>
#include <qcolor.h>
#include <qfont.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QKeyEvent>
#include <KoRuler.h> // for KoTabulatorList
#include <KoTextObject.h> // for KoTextView
#include <koffice_export.h>
class KoBgSpellCheck;
class KoTextObject;
class KoTextDocument;
class KoTextParag;
class KoTextFormat;
class KoParagCounter;
class KCommand;
class QTimer;
class KAction;
class KInstance;
class KDataToolInfo;
class KoLinkVariable;
class KoVariable;
class KoTextViewIface;
#include "KoRichText.h"
#include <QClipboard>
class KoBorder;
class KoParagStyle;

/**
 * Object that is created to edit a KoTextObject.
 * It handles all the events for it (mouse, keyboard).
 * There can be several KoTextView instances for the same KoTextObject.
 */
class KOTEXT_EXPORT KoTextView : public QObject, public KoTextFormatInterface
{
    Q_OBJECT
public:
    /**
     * Constructor. A KoTextView is associated to a KoTextObject.
     * Don't forget to call updateUI(true,true) in your derived constructor
     */
    KoTextView( KoTextObject *textobj );
    virtual ~KoTextView();

    void setBackSpeller( KoBgSpellCheck* backSpeller );

    virtual KoTextViewIface* dcopObject();

    /** Call this before deleting */
    /** don't remove selection when we made dnd between different frame*/
    void terminate( bool removeselection=true );

    KoTextObject * textObject() const { return m_textobj; }
    KoTextCursor * cursor() const { return m_cursor; }
    void setCursor( const KoTextCursor& cursor ) { *m_cursor = cursor; };
    KoTextDocument * textDocument() const;

    /** Return true if the view is allowed to modify the text object.
        This is the case by default */
    bool isReadWrite() const { return m_bReadWrite; }
    /** Call setReadWrite(false) to make the text view readonly */
    void setReadWrite( bool b ) { m_bReadWrite = b; }

    virtual KoTextFormat * currentFormat() const { return m_currentFormat; }
    void setCurrentFormat( KoTextFormat *fmt ) { m_currentFormat = fmt; }

    /**
     * Use this format for displaying the properties (Align/counter/...) of the object
     */
    virtual const KoParagLayout * currentParagLayoutFormat() const;

    virtual bool rtl() const;

    virtual KCommand *setChangeCaseOfTextCommand( KoChangeCaseDia::TypeOfCase _type );

    virtual KCommand* setParagLayoutFormatCommand( KoParagLayout *newLayout, int flags, int marginIndex = -1 );

    /** Implement the KoTextFormatInterface */
    virtual KCommand* setFormatCommand( const KoTextFormat * newFormat, int flags, bool zoomFont = false );

    // -- Paragraph settings --
    KCommand * setCounterCommand( const KoParagCounter & counter );
    KCommand * setAlignCommand( int align );
    KCommand * setPageBreakingCommand( int pageBreaking );
    KCommand * setLineSpacingCommand( double spacing, KoParagLayout::SpacingType _type );
    KCommand * setBordersCommand( const KoBorder& leftBorder, const KoBorder& rightBorder, const KoBorder& bottomBorder, const KoBorder& topBorder );
    KCommand * setJoinBordersCommand( bool join );
    KCommand * setMarginCommand( Q3StyleSheetItem::Margin m, double margin );
    KCommand * setTabListCommand( const KoTabulatorList & tabList );
    KCommand * setBackgroundColorCommand( const QColor & color );
    void applyStyle( const KoParagStyle * style );

    void dragStarted();
    void focusInEvent();
    void focusOutEvent();
    void handleKeyPressEvent( QKeyEvent * e, QWidget *, const QPoint& );
    void handleKeyReleaseEvent( QKeyEvent * e );
    void handleInputMethodEvent( QInputMethodEvent * e );
    // iPoint is in Layout Unit pixels
    // return true if we add new parag with "insert direct cursor"
    bool handleMousePressEvent( QMouseEvent* e, const QPoint& iPoint, bool canStartDrag = true, bool insertDirectCursor = false );
    void handleMouseMoveEvent( QMouseEvent* e, const QPoint& iPoint );
    void handleMouseReleaseEvent();
    void handleMouseDoubleClickEvent( QMouseEvent* e, const QPoint& iPoint );
    void handleMouseTripleClickEvent( QMouseEvent* e, const QPoint& /* Currently unused */ );
    bool maybeStartDrag( QMouseEvent* e );

    KoTextCursor selectWordUnderCursor( const KoTextCursor& cursor, int selectionId = KoTextDocument::Standard );
    KoTextCursor selectParagUnderCursor( const KoTextCursor& cursor, int selectionId = KoTextDocument::Standard, bool copyAndNotify = true );
    void extendParagraphSelection( const QPoint& iPoint );

    QString wordUnderCursor( const KoTextCursor& cursor );

    /** Return the list of actions from data-tools. Used to populate a RMB popupmenu usually. */
    QList<KAction *> dataToolActionList( KInstance * instance, const QString& word, bool & _singleWord );

    void insertSoftHyphen();
    void insertLineBreak();
    void insertNonbreakingSpace();
    void insertNonbreakingHyphen();
    void increaseNumberingLevel( const KoStyleCollection* styleCollection );
    void decreaseNumberingLevel( const KoStyleCollection* styleCollection );
    void insertSpecialChar( QChar _c, const QString& font );
    void changeCaseOfText( KoChangeCaseDia::TypeOfCase _type );

    void addBookmarks( const QString& );

    //return a pointer to the variable under the cursor, if any
    KoVariable *variable();
    //return a pointer to the link variable under the cursor, if any
    // (special case of variable())
    KoLinkVariable *linkVariable();

    /// A "drop move" will move the selected text to the given cursor position.
    /// This method only takes care of "removing the selected text"
    /// if the drop cursor isn't in the selected text (and !protectContent).
    KCommand *prepareDropMove( KoTextCursor dropCursor );

    void removeComment();
    void copyTextOfComment();

    // This is in fact "from selection or cursor"
    KoParagStyle * createStyleFromSelection( const QString & name );
    void updateStyleFromSelection( KoParagStyle* style );

    QString currentWordOrSelection() const;

    virtual void removeToolTipCompletion() {}

    // return true if we "insert direct cursor" and we insert new parag
    bool placeCursor( const QPoint &pos /* in internal coordinates */, bool insertDirectCursor=false );
    void setOverwriteMode( bool overwriteMode );

public slots:
    /** Show the current settings (those for the paragraph and character under the cursor), in the GUI.
     * The default implementation handles m_currentFormat and calls showCurrentFormat().
     * If you reimplement, make sure to call KoTextView::updateUI(updateFormat,force); */
    virtual void updateUI( bool updateFormat, bool force = false );
    virtual void ensureCursorVisible() = 0;
    void showCurrentFormat();

    // This allows KoTextObject to hide/show all the cursors before modifying anything
    void hideCursor() { drawCursor( false ); }
    void showCursor() { drawCursor( true ); }

    /** This is a slot so that it's accessible from DCOP */
    void insertText( const QString &text );
    void newParagraph();

    void copyLink();
    void removeLink();
    void completion();

    void setCursor( KoTextCursor * _cursor ) { *m_cursor = *_cursor; }

protected slots:
    /** Start a drag */
    virtual void startDrag() = 0;
    void slotToolActivated( const KDataToolInfo & info, const QString & command );
signals:
    void copy(QClipboard::Mode mode);
    void cut();
    void paste();

protected:
    /**
     * Called when a character (@p ch) has been inserted into @p parag, at the given @p index.
     * This is a virtual method rather than a signal for performance reasons.
     */
    virtual void doAutoFormat( KoTextCursor* /*cursor*/, KoTextParag * /*parag*/,
                               int /*index*/, QChar /*ch*/ ) { }

    virtual bool doCompletion( KoTextCursor* , KoTextParag *, int  ) { return false; }
    virtual bool doToolTipCompletion( KoTextCursor* , KoTextParag *, int, int  ) { return false; }
    virtual void showToolTipBox( KoTextParag *, int , QWidget *, const QPoint& ) {}

    virtual void textIncreaseIndent() {}
    virtual bool textDecreaseIndent() { return true; }

    //return true if we are a doubleSpace
    virtual bool doIgnoreDoubleSpace(KoTextParag * /*parag*/,
        int /*index*/,QChar /*ch*/ ) { return false;}

    /** Show the settings of this format in the GUI. Needs to be implemented in the application. */
    virtual void showFormat( KoTextFormat *format ) = 0;

    /** Draws the cursor (or hides it if b is false).
     * The default implementation only changes an internal flag, make sure to reimplement
     * and to call the parent implementation (in all cases)
     */
    virtual void drawCursor( bool b );

    /// Called when clicking on a link
    virtual bool openLink( KoLinkVariable* linkVariable );

    /** Reimplement this to handle PageUp. Example implementation:
        textView->cursor()->gotoPageUp( scrollview->visibleHeight() ); */
    virtual bool pgUpKeyPressed() = 0;
    /** Reimplement this to handle PageDown. Example implementation:
        textView->cursor()->gotoPageDown( scrollview->visibleHeight() ); */
    virtual bool pgDownKeyPressed() = 0;
    /** Reimplement this to handle CTRL+PageUp. Default implementation calls pgUpKeyPressed */
    virtual void ctrlPgUpKeyPressed() { pgUpKeyPressed(); }
    /** Reimplement this to handle CTRL+PageDown. Default implementation calls pgDownKeyPressed */
    virtual void ctrlPgDownKeyPressed() { pgDownKeyPressed(); }

    void deleteWordLeft();
    void deleteWordRight();
    bool insertParagraph( const QPoint &pos );

private slots:
    void blinkCursor();
    void tripleClickTimeout();
    void afterTripleClickTimeout();
protected:
    KoTextViewIface *dcop;
 public: // necessary to be public to allow script action in KoTextViewIface
    enum CursorAction { // keep in sync with QTextEdit
        MoveBackward,
        MoveForward,
        MoveWordBackward,
        MoveWordForward,
        MoveUp,
        MoveDown,
        MoveLineStart,
        MoveLineEnd,
        MoveHome,
        MoveEnd,
        MovePgUp,
        MovePgDown,
        MoveParagUp, // libkotext-specific
        MoveParagDown, // libkotext-specific
        MoveViewportUp, // KWord-specific
        MoveViewportDown // KWord-specific
    };

    void moveCursor( CursorAction action, bool select );
    bool moveCursor( CursorAction action );

private:
    KoTextObject *m_textobj;
    KoTextCursor *m_cursor;

    // Store the index of the variable on which we last clicked, to position m_cursor
    int m_variablePosition;
    bool m_overwriteMode;

    KoTextFormat *m_currentFormat;
    QTimer *blinkTimer, *dragStartTimer;
    class KoTextViewPrivate;
    KoTextViewPrivate *d;
    QPoint dragStartPos;
    bool m_cursorVisible;
    bool blinkCursorVisible;
    bool inDoubleClick;
    bool mightStartDrag;
    bool m_bReadWrite;
    bool possibleTripleClick;
    bool afterTripleClick;

    bool m_singleWord;
    QString m_wordUnderCursor;
};

#endif
