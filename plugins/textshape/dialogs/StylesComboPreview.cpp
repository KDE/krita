/* This file is part of the KDE project
 * Copyright (C) 2011 Pierre Stirnweiss <pstirnweiss@googlemail.com>
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

 * This class is inspired/ripped of KLineEdit, so here goes credit where credit is due (from klineedit.h):
 *  This class was originally inspired by Torben Weis'
 *  fileentry.cpp for KFM II.

 *  Copyright (C) 1997 Sven Radej <sven.radej@iname.com>
 *  Copyright (c) 1999 Patrick Ward <PAT_WARD@HP-USA-om5.om.hp.com>
 *  Copyright (c) 1999 Preston Brown <pbrown@kde.org>

 *  Completely re-designed:
 *  Copyright (c) 2000,2001 Dawit Alemayehu <adawit@kde.org>

 */
#include "StylesComboPreview.h"

#include <QModelIndex>
#include <QPainter>
#include <QPaintEvent>
#include <QFocusEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QPixmap>
#include <QPushButton>
#include <QString>

#include <KIcon>
#include <KLocale>

#include <KDebug>

/*
#include "klineedit.h"
#include "klineedit_p.h"

#include <kaction.h>
#include <kapplication.h>
#include <kauthorized.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kcompletionbox.h>
#include <kicontheme.h>
#include <kicon.h>
#include <klocale.h>
#include <kmenu.h>
#include <kstandardaction.h>
#include <kstandardshortcut.h>

#include <QtCore/QTimer>
#include <QtGui/QClipboard>
#include <QtGui/QStyleOption>
#include <QtGui/QToolTip>
*/

StylesComboPreview::StylesComboPreview(QWidget *parent) :
    QLineEdit(parent),
    m_clickInAdd(false),
    m_renamingNewStyle(false),
    m_shouldAddNewStyle(false),
    m_wideEnoughForAdd(true),
    m_addButton(0)
{
    init();
}

StylesComboPreview::~StylesComboPreview()
{
    delete m_addButton;
    m_addButton = 0;
}

void StylesComboPreview::init()
{
    setReadOnly(true);
    if (m_addButton) {
        return;
    }

    m_addButton = new QPushButton(this);
    m_addButton->setCursor( Qt::ArrowCursor );
    m_addButton->setIcon(KIcon("list-add"));
    m_addButton->setFlat(true);
    m_addButton->setMinimumSize(16,16);
    m_addButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_addButton->setMaximumSize(size().height(), size().height());
    connect(m_addButton, SIGNAL(clicked()), this, SLOT(addNewStyle()));
//        m_addButton->setToolTip( i18nc( "@action:button Clear current text in the line edit", "Clear text" ) );

    updateAddButtonIcon();
    updateAddButton();
//    connect(this, SIGNAL(textChanged(QString)), this, SLOT(updateAddButtonIcon(QString)));
}

void StylesComboPreview::setAddButtonShown(bool show)
{
    m_addButton->setVisible(show);
    /*
    if (show) {
        if (m_addButton) {
            return;
        }

        m_addButton = new QPushButton(this);
        m_addButton->setCursor( Qt::ArrowCursor );
        m_addButton->setIcon(KIcon("list-add"));
        m_addButton->setFlat(true);
        m_addButton->setMinimumSize(16,16);
        m_addButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        m_addButton->setMaximumSize(size().height(), size().height());
//        m_addButton->setToolTip( i18nc( "@action:button Clear current text in the line edit", "Clear text" ) );

        updateAddButtonIcon();
        updateAddButton();
        connect(this, SIGNAL(textChanged(QString)), this, SLOT(updateAddButtonIcon(QString)));
    } else {
        disconnect(this, SIGNAL(textChanged(QString)), this, SLOT(updateAddButtonIcon(QString)));
        delete m_addButton;
        m_addButton = 0;
        m_clickInAdd = false;
//        if (d->style) {
//            d->style.data()->m_overlap = 0;
//        }
    }
*/
}

QSize StylesComboPreview::availableSize() const
{
    return QSize(contentsRect().width()-m_addButton->width(), contentsRect().height());
}

void StylesComboPreview::setPreview(QPixmap pixmap)
{
    m_stylePreview = pixmap;
}

bool StylesComboPreview::isAddButtonShown() const
{
    return m_addButton != 0;
}

QSize StylesComboPreview::addButtonUsedSize() const
{
    QSize s;
    if (m_addButton) {
//        const int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth, 0, this);
        s = m_addButton->size();
//        s.rwidth() += frameWidth;
    }
    return s;
}

// Decides whether to show or hide the icon; called when the text changes
void StylesComboPreview::updateAddButtonIcon()
{
    if (!m_addButton) {
        return;
    }
//    if (isReadOnly()) {
//        d->adjustForReadOnly();
//        return;
//    }

//    int clearButtonState = KIconLoader::DefaultState;
/*
    if (d->wideEnoughForClear && text.length() > 0) {
        d->clearButton->animateVisible(true);
    } else {
        d->clearButton->animateVisible(false);
    }

    if (!d->clearButton->pixmap().isNull()) {
        return;
    }

    if (layoutDirection() == Qt::LeftToRight) {
        d->clearButton->setPixmap(SmallIcon("edit-clear-locationbar-rtl", 0, clearButtonState));
    } else {
        d->clearButton->setPixmap(SmallIcon("edit-clear-locationbar-ltr", 0, clearButtonState));
    }
*/
//    m_addButton->setVisible(true);
//    m_addButton->show();

}

// Determine geometry of clear button. Called initially, and on resizeEvent.
void StylesComboPreview::updateAddButton()
{
    if (!m_addButton) {
        return;
    }
//    if (isReadOnly()) {
//        d->adjustForReadOnly();
//        return;
//    }

    const QSize geom = size();
//    const int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth,0,this);
    const int buttonWidth = m_addButton->size().width();
//    const QSize newButtonSize(buttonWidth, geom.height());
/*
    const QFontMetrics fm(font());
    const int em = fm.width("m");

    // make sure we have enough room for the clear button
    // no point in showing it if we can't also see a few characters as well
    const bool wideEnough = geom.width() > 4 * em + buttonWidth + frameWidth;

    if (newButtonSize != m_addButton->size()) {
        m_addButton->resize(newButtonSize);
    }

    if (d->style) {
        d->style.data()->m_overlap = wideEnough ? buttonWidth + frameWidth : 0;
    }

    if (layoutDirection() == Qt::LeftToRight ) {
*/
        m_addButton->move(geom.width() /*- frameWidth*/ - buttonWidth , (geom.height()-m_addButton->height())/2);
/*    } else {
        d->clearButton->move(frameWidth + 1, 0);
    }

    if (wideEnough != d->wideEnoughForClear) {
        // we may (or may not) have been showing the button, but now our
        // positiong on that matter has shifted, so let's ensure that it
        // is properly visible (or not)
        d->wideEnoughForClear = wideEnough;
        updateClearButtonIcon(text());
    }
*/
}

void StylesComboPreview::resizeEvent( QResizeEvent * ev )
{

    updateAddButton();
    QLineEdit::resizeEvent(ev);
}

void StylesComboPreview::keyPressEvent( QKeyEvent *e )
{
    /*
    const int key = e->key() | e->modifiers();

    if ( KStandardShortcut::copy().contains( key ) )
    {
        copy();
        return;
    }
    else if ( KStandardShortcut::paste().contains( key ) )
    {
      // TODO:
      // we should restore the original text (not autocompleted), otherwise the paste
      // will get into troubles Bug: 134691
        if( !isReadOnly() )
          paste();
        return;
    }
    else if ( KStandardShortcut::pasteSelection().contains( key ) )
    {
        QString text = QApplication::clipboard()->text( QClipboard::Selection);
        insert( text );
        deselect();
        return;
    }

    else if ( KStandardShortcut::cut().contains( key ) )
    {
        if( !isReadOnly() )
           cut();
        return;
    }
    else if ( KStandardShortcut::undo().contains( key ) )
    {
        if( !isReadOnly() )
          undo();
        return;
    }
    else if ( KStandardShortcut::redo().contains( key ) )
    {
        if( !isReadOnly() )
           redo();
        return;
    }
    else if ( KStandardShortcut::deleteWordBack().contains( key ) )
    {
        cursorWordBackward(true);
        if ( hasSelectedText() )
            del();

        e->accept();
        return;
    }
    else if ( KStandardShortcut::deleteWordForward().contains( key ) )
    {
        // Workaround for QT bug where
        cursorWordForward(true);
        if ( hasSelectedText() )
            del();

        e->accept();
        return;
    }
    else if ( KStandardShortcut::backwardWord().contains( key ) )
    {
      cursorWordBackward(false);
      e->accept();
      return;
    }
    else if ( KStandardShortcut::forwardWord().contains( key ) )
    {
      cursorWordForward(false);
      e->accept();
      return;
    }
    else if ( KStandardShortcut::beginningOfLine().contains( key ) )
    {
      home(false);
      e->accept();
      return;
    }
    else if ( KStandardShortcut::endOfLine().contains( key ) )
    {
      end(false);
      e->accept();
      return;
    }


    // Filter key-events if EchoMode is normal and
    // completion mode is not set to CompletionNone
    if ( echoMode() == QLineEdit::Normal &&
         completionMode() != KGlobalSettings::CompletionNone )
    {
        const KeyBindingMap keys = getKeyBindings();
        const KGlobalSettings::Completion mode = completionMode();
        const bool noModifier = (e->modifiers() == Qt::NoButton ||
                           e->modifiers() == Qt::ShiftModifier ||
                           e->modifiers() == Qt::KeypadModifier);

        if ( (mode == KGlobalSettings::CompletionAuto ||
              mode == KGlobalSettings::CompletionPopupAuto ||
              mode == KGlobalSettings::CompletionMan) && noModifier )
        {
            if ( !d->userSelection && hasSelectedText() &&
                 ( e->key() == Qt::Key_Right || e->key() == Qt::Key_Left ) &&
                 e->modifiers()==Qt::NoButton )
            {
                const QString old_txt = text();
                d->disableRestoreSelection = true;
                const int start = selectionStart();

                deselect();
                QLineEdit::keyPressEvent ( e );
                const int cPosition=cursorPosition();
                setText(old_txt);

                // keep cursor at cPosition
                setSelection(old_txt.length(), cPosition - old_txt.length());
                if (e->key() == Qt::Key_Right && cPosition > start )
                {
                    //the user explicitly accepted the autocompletion
                    d->_k_updateUserText(text());
                }

                d->disableRestoreSelection = false;
                return;
            }

            if ( e->key() == Qt::Key_Escape )
            {
                if (hasSelectedText() && !d->userSelection )
                {
                    del();
                    setUserSelection(true);
                }

                // Don't swallow the Escape press event for the case
                // of dialogs, which have Escape associated to Cancel
                e->ignore();
                return;
            }

        }

        if ( (mode == KGlobalSettings::CompletionAuto ||
              mode == KGlobalSettings::CompletionMan) && noModifier )
        {
            const QString keycode = e->text();
            if ( !keycode.isEmpty() && (keycode.unicode()->isPrint() ||
                e->key() == Qt::Key_Backspace || e->key() == Qt::Key_Delete ) )
            {
                const bool hasUserSelection=d->userSelection;
                const bool hadSelection=hasSelectedText();

                bool cursorNotAtEnd=false;

                const int start = selectionStart();
                const int cPos = cursorPosition();

                // When moving the cursor, we want to keep the autocompletion as an
                // autocompletion, so we want to process events at the cursor position
                // as if there was no selection. After processing the key event, we
                // can set the new autocompletion again.
                if ( hadSelection && !hasUserSelection && start>cPos )
                {
                    del();
                    setCursorPosition(cPos);
                    cursorNotAtEnd=true;
                }

                d->disableRestoreSelection = true;
                QLineEdit::keyPressEvent ( e );
                d->disableRestoreSelection = false;

                QString txt = text();
                int len = txt.length();
*/
//                if ( !hasSelectedText() && len /*&& cursorPosition() == len */)
/*                {
                    if ( e->key() == Qt::Key_Backspace )
                    {
                        if ( hadSelection && !hasUserSelection && !cursorNotAtEnd )
                        {
                            backspace();
                            txt = text();
                            len = txt.length();
                        }

                        if (!d->s_backspacePerformsCompletion || !len) {
                            d->autoSuggest = false;
                        }
                    }

                    if (e->key() == Qt::Key_Delete )
                        d->autoSuggest=false;

                    doCompletion(txt);

                    if(  (e->key() == Qt::Key_Backspace || e->key() == Qt::Key_Delete) )
                        d->autoSuggest=true;

                    e->accept();
                }

                return;
            }

        }

        else if (( mode == KGlobalSettings::CompletionPopup ||
                   mode == KGlobalSettings::CompletionPopupAuto ) &&
                   noModifier && !e->text().isEmpty() )
        {
            const QString old_txt = text();
            const bool hasUserSelection=d->userSelection;
            const bool hadSelection=hasSelectedText();
            bool cursorNotAtEnd=false;

            const int start = selectionStart();
            const int cPos = cursorPosition();
            const QString keycode = e->text();

            // When moving the cursor, we want to keep the autocompletion as an
            // autocompletion, so we want to process events at the cursor position
            // as if there was no selection. After processing the key event, we
            // can set the new autocompletion again.
            if (hadSelection && !hasUserSelection && start>cPos &&
               ( (!keycode.isEmpty() && keycode.unicode()->isPrint()) ||
                 e->key() == Qt::Key_Backspace || e->key() == Qt::Key_Delete ) )
            {
                del();
                setCursorPosition(cPos);
                cursorNotAtEnd=true;
            }

            const int selectedLength=selectedText().length();

            d->disableRestoreSelection = true;
            QLineEdit::keyPressEvent ( e );
            d->disableRestoreSelection = false;

            if (( selectedLength != selectedText().length() ) && !hasUserSelection )
                slotRestoreSelectionColors(); // and set userSelection to true

            QString txt = text();
            int len = txt.length();
*/
//            if ( ( txt != old_txt || txt != e->text() ) && len/* && ( cursorPosition() == len || force )*/ &&
//                 ( (!keycode.isEmpty() && keycode.unicode()->isPrint()) ||
//                   e->key() == Qt::Key_Backspace || e->key() == Qt::Key_Delete) )
/*            {
                if ( e->key() == Qt::Key_Backspace )
                {
                    if ( hadSelection && !hasUserSelection && !cursorNotAtEnd )
                    {
                        backspace();
                        txt = text();
                        len = txt.length();
                    }

                    if (!d->s_backspacePerformsCompletion) {
                        d->autoSuggest = false;
                    }
                }

                if (e->key() == Qt::Key_Delete )
                    d->autoSuggest=false;

                if ( d->completionBox )
                  d->completionBox->setCancelledText( txt );

                doCompletion(txt);

                if ( (e->key() == Qt::Key_Backspace || e->key() == Qt::Key_Delete ) &&
                    mode == KGlobalSettings::CompletionPopupAuto )
                  d->autoSuggest=true;

                e->accept();
            }
            else if (!len && d->completionBox && d->completionBox->isVisible())
                d->completionBox->hide();

            return;
        }

        else if ( mode == KGlobalSettings::CompletionShell )
        {
            // Handles completion.
            KShortcut cut;
            if ( keys[TextCompletion].isEmpty() )
                cut = KStandardShortcut::shortcut(KStandardShortcut::TextCompletion);
            else
                cut = keys[TextCompletion];

            if ( cut.contains( key ) )
            {
                // Emit completion if the completion mode is CompletionShell
                // and the cursor is at the end of the string.
                const QString txt = text();
                const int len = txt.length();
                if ( cursorPosition() == len && len != 0 )
                {
                    doCompletion(txt);
                    return;
                }
            }
            else if ( d->completionBox )
                d->completionBox->hide();
        }

        // handle rotation
        if ( mode != KGlobalSettings::CompletionNone )
        {
            // Handles previous match
            KShortcut cut;
            if ( keys[PrevCompletionMatch].isEmpty() )
                cut = KStandardShortcut::shortcut(KStandardShortcut::PrevCompletion);
            else
                cut = keys[PrevCompletionMatch];

            if ( cut.contains( key ) )
            {
                if ( emitSignals() )
                    emit textRotation( KCompletionBase::PrevCompletionMatch );
                if ( handleSignals() )
                    rotateText( KCompletionBase::PrevCompletionMatch );
                return;
            }

            // Handles next match
            if ( keys[NextCompletionMatch].isEmpty() )
                cut = KStandardShortcut::shortcut(KStandardShortcut::NextCompletion);
            else
                cut = keys[NextCompletionMatch];

            if ( cut.contains( key ) )
            {
                if ( emitSignals() )
                    emit textRotation( KCompletionBase::NextCompletionMatch );
                if ( handleSignals() )
                    rotateText( KCompletionBase::NextCompletionMatch );
                return;
            }
        }

        // substring completion
        if ( compObj() )
        {
            KShortcut cut;
            if ( keys[SubstringCompletion].isEmpty() )
                cut = KStandardShortcut::shortcut(KStandardShortcut::SubstringCompletion);
            else
                cut = keys[SubstringCompletion];

            if ( cut.contains( key ) )
            {
                if ( emitSignals() )
                    emit substringCompletion( text() );
                if ( handleSignals() )
                {
                    setCompletedItems( compObj()->substringCompletion(text()));
                    e->accept();
                }
                return;
            }
        }
    }
    const int selectedLength = selectedText().length();

    // Let QLineEdit handle any other keys events.
    QLineEdit::keyPressEvent ( e );

    if ( selectedLength != selectedText().length() )
        slotRestoreSelectionColors(); // and set userSelection to true
*/
    if (e->key() == Qt::Key_Escape) {
        m_renamingNewStyle = false;
        m_shouldAddNewStyle = false;
        setReadOnly(true);
        setText(QString());
        e->accept();
    }
    else {
        QLineEdit::keyPressEvent(e);
    }
}

void StylesComboPreview::addNewStyle()
{
    m_renamingNewStyle = true;
    m_shouldAddNewStyle = true;
    setText(i18n("New style"));
    selectAll();
    setReadOnly(false);
    this->setFocus();
}

void StylesComboPreview::mousePressEvent( QMouseEvent* e )
{
    /*
    kDebug() << "in buttonPressed";
    kDebug() << "m_addButton: " << m_addButton;
    if  ((e->button() == Qt::LeftButton ||
           e->button() == Qt::MidButton) &&
          m_addButton) {
        m_clickInAdd = m_addButton == childAt(e->pos());
        kDebug() << "mousePressed in addButton";
    }
*/
    QLineEdit::mousePressEvent( e );
}

void StylesComboPreview::mouseReleaseEvent( QMouseEvent* e )
{
/*
    kDebug() << "in mouseReleased. clickinAdd: " << m_clickInAdd;
    if (m_clickInAdd) {
        if (m_addButton == childAt(e->pos())) {
            kDebug() << "mouseReleased in addButton";
            QString newText;
            if ( e->button() == Qt::MidButton ) {
                newText = QApplication::clipboard()->text( QClipboard::Selection );
                setText( newText );
            } else {
                setSelection(0, text().size());
                del();
                emit clearButtonClicked();
            }
            emit textChanged( newText );
        }

        d->clickInClear = false;
        e->accept();
        return;

        }
    }
*/
    QLineEdit::mouseReleaseEvent( e );

/*   if (QApplication::clipboard()->supportsSelection() ) {
       if ( e->button() == Qt::LeftButton ) {
            // Fix copying of squeezed text if needed
            copySqueezedText( false );
       }
   }
*/
}

bool StylesComboPreview::event( QEvent* ev )
{
    /*
    KCursor::autoHideEventFilter( this, ev );
    if ( ev->type() == QEvent::ShortcutOverride )
    {
        QKeyEvent *e = static_cast<QKeyEvent *>( ev );
        if (d->overrideShortcut(e)) {
            ev->accept();
        }
    } else if( ev->type() == QEvent::KeyPress ) {
        // Hmm -- all this could be done in keyPressEvent too...

        QKeyEvent *e = static_cast<QKeyEvent *>( ev );

        if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
            const bool trap = d->completionBox && d->completionBox->isVisible();

            const bool stopEvent = trap || (d->grabReturnKeyEvents &&
                                      (e->modifiers() == Qt::NoButton ||
                                       e->modifiers() == Qt::KeypadModifier));

            // Qt will emit returnPressed() itself if we return false
            if (stopEvent) {
                emit QLineEdit::returnPressed();
                e->accept();
            }

            emit returnPressed( displayText() );

            if (trap) {
                d->completionBox->hide();
                deselect();
                setCursorPosition(text().length());
            }

            // Eat the event if the user asked for it, or if a completionbox was visible
            if (stopEvent) {
                return true;
            }
        }
    } else if (ev->type() == QEvent::ApplicationPaletteChange
               || ev->type() == QEvent::PaletteChange) {
        // Assume the widget uses the application's palette
        QPalette p = QApplication::palette();
        d->previousHighlightedTextColor=p.color(QPalette::Normal,QPalette::HighlightedText);
        d->previousHighlightColor=p.color(QPalette::Normal,QPalette::Highlight);
        setUserSelection(d->userSelection);
    } else if (ev->type() == QEvent::StyleChange) {
        // since we have our own style and it relies on this style to Get Things Right,
        // if a style is set specifically on the widget (which would replace our own style!)
        // hang on to this special style and re-instate our own style.
        //FIXME: Qt currently has a grave bug where already deleted QStyleSheetStyle objects
        // will get passed back in if we set a new style on it here. remove the qstrmcp test
        // when this is fixed in Qt (or a better approach is found)
        if (!qobject_cast<KLineEditStyle *>(style()) &&
            qstrcmp(style()->metaObject()->className(), "QStyleSheetStyle") != 0 &&
            QLatin1String(style()->metaObject()->className()) != d->lastStyleClass) {
            KLineEditStyle *kleStyle = d->style.data();
            if (!kleStyle) {
                d->style = kleStyle = new KLineEditStyle(this);
            }

            kleStyle->m_subStyle = style();
            // this guards against "wrap around" where another style, e.g. QStyleSheetStyle,
            // is setting the style on QEvent::StyleChange
            d->lastStyleClass = QLatin1String(style()->metaObject()->className());
            setStyle(kleStyle);
            d->lastStyleClass.clear();
        }
    }
*/
    if (ev->type() == QEvent::FocusOut) {
        QFocusEvent *focusEvent = static_cast<QFocusEvent *>(ev);
        if (focusEvent->reason() != Qt::ActiveWindowFocusReason && focusEvent->reason() != Qt::PopupFocusReason) {
            if (m_shouldAddNewStyle) {
                emit newStyleRequested(text());
                setReadOnly(true);
                m_renamingNewStyle = false;
                setText(QString());
                return true;
            }
            setReadOnly(true);
            m_renamingNewStyle = false;
            setText(QString());
        }
    }
    return QLineEdit::event( ev );
}

void StylesComboPreview::paintEvent( QPaintEvent *ev )
{
//    QLineEdit::paintEvent(ev);
//    p.fillRect(this->contentsRect(),Qt::white);
//    QRect pixmapRect(contentsRect().left(), contentsRect().top(), contentsRect().width()-m_addButton->width(), contentsRect().height());
    if (!m_renamingNewStyle) {
        QLineEdit::paintEvent(ev);
        QPainter p(this);
        p.setClipRect(ev->rect());
        p.drawPixmap(contentsRect().topLeft(), m_stylePreview);
    }
    else {
        QLineEdit::paintEvent(ev);
    }
    /*
    if (echoMode() == Password && d->threeStars) {
        // ### hack alert!
        // QLineEdit has currently no hooks to modify the displayed string.
        // When we call setText(), an update() is triggered and we get
        // into an infinite recursion.
        // Qt offers the setUpdatesEnabled() method, but when we re-enable
        // them, update() is triggered, and we get into the same recursion.
        // To work around this problem, we set/clear the internal Qt flag which
        // marks the updatesDisabled state manually.
        setAttribute(Qt::WA_UpdatesDisabled, true);
        blockSignals(true);
        const QString oldText = text();
        const bool isModifiedState = isModified(); // save modified state because setText resets it
        setText(oldText + oldText + oldText);
        QLineEdit::paintEvent(ev);
        setText(oldText);
        setModified(isModifiedState);
        blockSignals(false);
        setAttribute(Qt::WA_UpdatesDisabled, false);
    } else {
        QLineEdit::paintEvent( ev );
    }

    if (d->enableClickMsg && d->drawClickMsg && !hasFocus() && text().isEmpty()) {
        QPainter p(this);
        QFont f = font();
        f.setItalic(d->italicizePlaceholder);
        p.setFont(f);

        QColor color(palette().color(foregroundRole()));
        color.setAlphaF(0.5);
        p.setPen(color);

        QStyleOptionFrame opt;
        initStyleOption(&opt);
        QRect cr = style()->subElementRect(QStyle::SE_LineEditContents, &opt, this);

        // this is copied/adapted from QLineEdit::paintEvent
        const int verticalMargin(1);
        const int horizontalMargin(2);

        int left, top, right, bottom;
        getTextMargins( &left, &top, &right, &bottom );
        cr.adjust( left, top, -right, -bottom );

        p.setClipRect(cr);

        QFontMetrics fm = fontMetrics();
        Qt::Alignment va = alignment() & Qt::AlignVertical_Mask;
        int vscroll;
        switch (va & Qt::AlignVertical_Mask)
        {
            case Qt::AlignBottom:
            vscroll = cr.y() + cr.height() - fm.height() - verticalMargin;
            break;

            case Qt::AlignTop:
            vscroll = cr.y() + verticalMargin;
            break;

            default:
            vscroll = cr.y() + (cr.height() - fm.height() + 1) / 2;
            break;

        }

        QRect lineRect(cr.x() + horizontalMargin, vscroll, cr.width() - 2*horizontalMargin, fm.height());
        p.drawText(lineRect, Qt::AlignLeft|Qt::AlignVCenter, d->clickMessage);

    }
*/
}

#include "StylesComboPreview.moc"
