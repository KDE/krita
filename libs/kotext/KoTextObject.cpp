/* This file is part of the KDE project
   Copyright (C) 2001-2006 David Faure <faure@kde.org>
   Copyright (C) 2005 Martin Ellis <martin.ellis@kdemail.net>

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

#include "KoTextObject.h"
#include "KoTextParag.h"
#include "KoParagCounter.h"
#include "KoTextZoomHandler.h"
#include "KoTextCommand.h"
#include "KoStyleCollection.h"
#include "KoFontDia.h"
#include "KoOasisContext.h"
#include "KoVariable.h"
#include "KoAutoFormat.h"
#include <KoXmlNS.h>
#include <KoDom.h>

#include <klocale.h>
#include <kdebug.h>
#include <kapplication.h>

#include <qtimer.h>
#include <qregexp.h>
#include <q3progressdialog.h>
//Added by qt3to4:
#include <Q3CString>
#include <Q3MemArray>
#include <Q3ValueList>

#include <assert.h>

//#define DEBUG_FORMATS
//#define DEBUG_FORMAT_MORE

const char KoTextObject::s_customItemChar = '#'; // Has to be transparent to kspell but still be saved (not space)

struct KoTextObject::KoTextObjectPrivate
{
public:
    KoTextObjectPrivate() {
        afterFormattingEmitted = false;
        abortFormatting = false;
    }
    bool afterFormattingEmitted;
    bool abortFormatting;
};

KoTextObject::KoTextObject( KoTextZoomHandler *zh, const QFont& defaultFont,
                            const QString &defaultLanguage, bool hyphenation,
                            KoParagStyle* defaultStyle, int tabStopWidth,
                            QObject* parent, const char *name )
    : QObject( parent, name ), m_defaultStyle( defaultStyle ), undoRedoInfo( this )
{
    textdoc = new KoTextDocument( zh, new KoTextFormatCollection( defaultFont, QColor(),defaultLanguage, hyphenation ) );
    if ( tabStopWidth != -1 )
        textdoc->setTabStops( tabStopWidth );
    init();
}

KoTextObject::KoTextObject( KoTextDocument* _textdoc, KoParagStyle* defaultStyle,
                            QObject* parent, const char *name )
 : QObject( parent, name ), m_defaultStyle( defaultStyle ), undoRedoInfo( this )
{
    textdoc = _textdoc;
    init();
}

void KoTextObject::init()
{
    d = new KoTextObjectPrivate;
    m_needsSpellCheck = true;
    m_protectContent = false;
    m_visible=true;
    m_availableHeight = -1;
    m_lastFormatted = textdoc->firstParag();
    m_highlightSelectionAdded = false;
    interval = 0;
    startTimer = new QTimer( this );
    connect( startTimer, SIGNAL( timeout() ),
             this, SLOT( doChangeInterval() ) );

    formatTimer = new QTimer( this );
    connect( formatTimer, SIGNAL( timeout() ),
             this, SLOT( formatMore() ) );

    // Apply default style to initial paragraph
    if ( m_lastFormatted && m_defaultStyle )
        m_lastFormatted->applyStyle( m_defaultStyle );

    connect( textdoc, SIGNAL( paragraphDeleted( KoTextParag* ) ),
             this, SIGNAL( paragraphDeleted( KoTextParag* ) ) );
    connect( textdoc, SIGNAL( paragraphDeleted( KoTextParag* ) ),
             this, SLOT( slotParagraphDeleted( KoTextParag* ) ) );
    connect( textdoc, SIGNAL( newCommand( KCommand* ) ),
             this, SIGNAL( newCommand( KCommand* ) ) );
    connect( textdoc, SIGNAL( repaintChanged() ),
             this, SLOT( emitRepaintChanged() ) );

    connect( this, SIGNAL(paragraphModified( KoTextParag*, int, int , int ) ),
             this, SLOT(slotParagraphModified(KoTextParag *, int, int , int)));
    connect( this, SIGNAL(paragraphCreated( KoTextParag* )),
             this, SLOT(slotParagraphCreated(KoTextParag *)));
}

KoTextObject::~KoTextObject()
{
    // Avoid crash in KoTextString::clear -> accessing deleted format collection,
    // if ~UndoRedoInfo still has a string to clear.
    undoRedoInfo.clear();
    delete textdoc; textdoc = 0;
    delete d;
}

int KoTextObject::availableHeight() const
{
    if ( m_availableHeight == -1 )
        emit const_cast<KoTextObject *>(this)->availableHeightNeeded();
    Q_ASSERT( m_availableHeight != -1 );
    return m_availableHeight;
}

void KoTextObject::slotParagraphModified(KoTextParag * /*parag*/, int /*ParagModifyType*/ _type, int , int)
{
    if ( _type == ChangeFormat)
        return;
    m_needsSpellCheck = true;
}

void KoTextObject::slotParagraphCreated(KoTextParag * /*parag*/)
{
    m_needsSpellCheck = true;
}

void KoTextObject::slotParagraphDeleted(KoTextParag * /*parag*/)
{
    // ### TODO: remove from kwbgspellcheck
    // not needed, since KoTextIterator takes care of that.
}

int KoTextObject::docFontSize( KoTextFormat * format ) const
{
    Q_ASSERT( format );
    return format->pointSize();
}

int KoTextObject::zoomedFontSize( int docFontSize ) const
{
    kDebug(32500) << "KoTextObject::zoomedFontSize: docFontSize=" << docFontSize
              << " - in LU: " << KoTextZoomHandler::ptToLayoutUnitPt( docFontSize ) << endl;
    return KoTextZoomHandler::ptToLayoutUnitPt( docFontSize );
}

// A visitor that looks for custom items in e.g. a selection
class KoHasCustomItemVisitor : public KoParagVisitor
{
public:
    KoHasCustomItemVisitor() : KoParagVisitor() { }
    // returns false when cancelled, i.e. an item was _found_, and true if it proceeded to the end(!)
    virtual bool visit( KoTextParag *parag, int start, int end )
    {
        for ( int i = start ; i < end ; ++i )
        {
            KoTextStringChar * ch = parag->at( i );
            if ( ch->isCustom() )
                return false; // found one -> stop here
        }
        return true;
    }
};

bool KoTextObject::selectionHasCustomItems( KoTextDocument::SelectionId selectionId ) const
{
    KoHasCustomItemVisitor visitor;
    bool noneFound = textdoc->visitSelection( selectionId, &visitor );
    return !noneFound;
}

void KoTextObject::slotAfterUndoRedo()
{
    formatMore( 2 );
    emit repaintChanged( this );
    emit updateUI( true );
    emit showCursor();
    emit ensureCursorVisible();
}

void KoTextObject::clearUndoRedoInfo()
{
    undoRedoInfo.clear();
}


void KoTextObject::checkUndoRedoInfo( KoTextCursor * cursor, UndoRedoInfo::Type t )
{
    if ( undoRedoInfo.valid() && ( t != undoRedoInfo.type || cursor != undoRedoInfo.cursor ) ) {
        undoRedoInfo.clear();
    }
    undoRedoInfo.type = t;
    undoRedoInfo.cursor = cursor;
}

void KoTextObject::undo()
{
    undoRedoInfo.clear();
    emit hideCursor();
    KoTextCursor *cursor = new KoTextCursor( textdoc ); // Kindof a dummy cursor
    KoTextCursor *c = textdoc->undo( cursor );
    if ( !c ) {
        delete cursor;
        emit showCursor();
        return;
    }
    // We have to set this new cursor position in all views :(
    // It sucks a bit for useability, but otherwise one view might still have
    // a cursor inside a deleted paragraph -> crash.
    emit setCursor( c );
    setLastFormattedParag( textdoc->firstParag() );
    delete cursor;
    QTimer::singleShot( 0, this, SLOT( slotAfterUndoRedo() ) );
}

void KoTextObject::redo()
{
    undoRedoInfo.clear();
    emit hideCursor();
    KoTextCursor *cursor = new KoTextCursor( textdoc ); // Kindof a dummy cursor
    KoTextCursor *c = textdoc->redo( cursor );
    if ( !c ) {
        delete cursor;
        emit showCursor();
        return;
    }
    emit setCursor( c ); // see undo
    setLastFormattedParag( textdoc->firstParag() );
    delete cursor;
    QTimer::singleShot( 0, this, SLOT( slotAfterUndoRedo() ) );
}

KoTextObject::UndoRedoInfo::UndoRedoInfo( KoTextObject *to )
    : type( Invalid ), textobj(to), cursor( 0 )
{
    text = QString::null;
    id = -1;
    index = -1;
    placeHolderCmd = 0L;
}

bool KoTextObject::UndoRedoInfo::valid() const
{
    return text.length() > 0 && id >= 0 && index >= 0;
}

void KoTextObject::UndoRedoInfo::clear()
{
    if ( valid() ) {
        KoTextDocument* textdoc = textobj->textDocument();
        switch (type) {
            case Insert:
            case Return:
            {
                KoTextDocCommand * cmd = new KoTextInsertCommand( textdoc, id, index, text.rawData(), customItemsMap, oldParagLayouts );
                textdoc->addCommand( cmd );
                Q_ASSERT( placeHolderCmd );
                // Inserting any custom items -> macro command, to let custom items add their command
                if ( !customItemsMap.isEmpty() )
                {
                    CustomItemsMap::Iterator it = customItemsMap.begin();
                    for ( ; it != customItemsMap.end(); ++it )
                    {
                        KoTextCustomItem * item = it.data();
                        KCommand * itemCmd = item->createCommand();
                        if ( itemCmd )
                            placeHolderCmd->addCommand( itemCmd );
                    }
                    placeHolderCmd->addCommand( new KoTextCommand( textobj, /*cmd, */QString::null ) );
                }
                else
                {
                    placeHolderCmd->addCommand( new KoTextCommand( textobj, /*cmd, */QString::null ) );
                }
            } break;
            case Delete:
            case RemoveSelected:
            {
                KoTextDocCommand * cmd = textobj->deleteTextCommand( textdoc, id, index, text.rawData(), customItemsMap, oldParagLayouts );
                textdoc->addCommand( cmd );
                Q_ASSERT( placeHolderCmd );
                placeHolderCmd->addCommand( new KoTextCommand( textobj, /*cmd, */QString::null ) );
                // Deleting any custom items -> let them add their command
                if ( !customItemsMap.isEmpty() )
                {
                    customItemsMap.deleteAll( placeHolderCmd );
                }
           } break;
            case Invalid:
                break;
        }
    }
    type = Invalid;
    // Before Qt-3.2.0, this called KoTextString::clear(), which called resize(0) on the array, which _detached_. Tricky.
    // Since Qt-3.2.0, resize(0) doesn't detach anymore -> KoTextDocDeleteCommand calls copy() itself.
    text = QString::null;
    id = -1;
    index = -1;
    oldParagLayouts.clear();
    customItemsMap.clear();
    placeHolderCmd = 0L;
}

void KoTextObject::copyCharFormatting( KoTextParag *parag, int position, int index /*in text*/, bool moveCustomItems )
{
    KoTextStringChar * ch = parag->at( position );
    if ( ch->format() ) {
        ch->format()->addRef();
        undoRedoInfo.text.at( index ).setFormat( ch->format() );
    }
    if ( ch->isCustom() )
    {
        kDebug(32500) << "KoTextObject::copyCharFormatting moving custom item " << ch->customItem() << " to text's " << index << " char"  << endl;
        undoRedoInfo.customItemsMap.insert( index, ch->customItem() );
        // We copy the custom item to customItemsMap in all cases (see setFormat)
        // We only remove from 'ch' if moveCustomItems was specified
        if ( moveCustomItems )
            parag->removeCustomItem(position);
        //ch->loseCustomItem();
    }
}

// Based on QTextView::readFormats - with all code duplication moved to copyCharFormatting
void KoTextObject::readFormats( KoTextCursor &c1, KoTextCursor &c2, bool copyParagLayouts, bool moveCustomItems )
{
    //kDebug(32500) << "KoTextObject::readFormats moveCustomItems=" << moveCustomItems << endl;
    int oldLen = undoRedoInfo.text.length();
    if ( c1.parag() == c2.parag() ) {
        undoRedoInfo.text += c1.parag()->string()->toString().mid( c1.index(), c2.index() - c1.index() );
        for ( int i = c1.index(); i < c2.index(); ++i )
            copyCharFormatting( c1.parag(), i, oldLen + i - c1.index(), moveCustomItems );
    } else {
        int lastIndex = oldLen;
        int i;
        //kDebug(32500) << "KoTextObject::readFormats copying from " << c1.index() << " to " << c1.parag()->length()-1 << " into lastIndex=" << lastIndex << endl;
        // Replace the trailing spaces with '\n'. That char carries the formatting for the trailing space.
        undoRedoInfo.text += c1.parag()->string()->toString().mid( c1.index(), c1.parag()->length() - 1 - c1.index() ) + '\n';
        for ( i = c1.index(); i < c1.parag()->length(); ++i, ++lastIndex )
            copyCharFormatting( c1.parag(), i, lastIndex, moveCustomItems );
        //++lastIndex; // skip the '\n'.
        KoTextParag *p = c1.parag()->next();
        while ( p && p != c2.parag() ) {
            undoRedoInfo.text += p->string()->toString().left( p->length() - 1 ) + '\n';
            //kDebug(32500) << "KoTextObject::readFormats (mid) copying from 0 to "  << p->length()-1 << " into i+" << lastIndex << endl;
            for ( i = 0; i < p->length(); ++i )
                copyCharFormatting( p, i, i + lastIndex, moveCustomItems );
            lastIndex += p->length(); // + 1; // skip the '\n'
            //kDebug(32500) << "KoTextObject::readFormats lastIndex now " << lastIndex << " - text is now " << undoRedoInfo.text.toString() << endl;
            p = p->next();
        }
        //kDebug(32500) << "KoTextObject::readFormats copying [last] from 0 to " << c2.index() << " into i+" << lastIndex << endl;
        undoRedoInfo.text += c2.parag()->string()->toString().left( c2.index() );
        for ( i = 0; i < c2.index(); ++i )
            copyCharFormatting( c2.parag(), i, i + lastIndex, moveCustomItems );
    }

    if ( copyParagLayouts ) {
        KoTextParag *p = c1.parag();
        while ( p ) {
            undoRedoInfo.oldParagLayouts << p->paragLayout();
            if ( p == c2.parag() )
                break;
            p = p->next();
        }
    }
}

void KoTextObject::newPlaceHolderCommand( const QString & name )
{
    Q_ASSERT( !undoRedoInfo.placeHolderCmd );
    if ( undoRedoInfo.placeHolderCmd ) kDebug(32500) << kBacktrace();
    undoRedoInfo.placeHolderCmd = new KMacroCommand( name );
    emit newCommand( undoRedoInfo.placeHolderCmd );
}

void KoTextObject::storeParagUndoRedoInfo( KoTextCursor * cursor, KoTextDocument::SelectionId selectionId )
{
    undoRedoInfo.clear();
    undoRedoInfo.oldParagLayouts.clear();
    undoRedoInfo.text = " ";
    undoRedoInfo.index = 1;
    if ( cursor && !textdoc->hasSelection( selectionId, true ) ) {
        KoTextParag * p = cursor->parag();
        undoRedoInfo.id = p->paragId();
        undoRedoInfo.eid = p->paragId();
        undoRedoInfo.oldParagLayouts << p->paragLayout();
    }
    else{
        Q_ASSERT( textdoc->hasSelection( selectionId, true ) );
        KoTextParag *start = textdoc->selectionStart( selectionId );
        KoTextParag *end = textdoc->selectionEnd( selectionId );
        undoRedoInfo.id = start->paragId();
        undoRedoInfo.eid = end->paragId();
        for ( ; start && start != end->next() ; start = start->next() )
        {
            undoRedoInfo.oldParagLayouts << start->paragLayout();
            //kDebug(32500) << "KoTextObject:storeParagUndoRedoInfo storing counter " << start->paragLayout().counter.counterType << endl;
        }
    }
}

void KoTextObject::doKeyboardAction( KoTextCursor * cursor, KoTextFormat * & /*currentFormat*/, KeyboardAction action )
{
    KoTextParag * parag = cursor->parag();
    setLastFormattedParag( parag );
    emit hideCursor();
    bool doUpdateCurrentFormat = true;
    switch ( action ) {
    case ActionDelete: {
        checkUndoRedoInfo( cursor, UndoRedoInfo::Delete );
        if ( !undoRedoInfo.valid() ) {
            newPlaceHolderCommand( i18n("Delete Text") );
            undoRedoInfo.id = parag->paragId();
            undoRedoInfo.index = cursor->index();
            undoRedoInfo.text = QString::null;
            undoRedoInfo.oldParagLayouts << parag->paragLayout();
        }
        if ( !cursor->atParagEnd() )
        {
            KoTextStringChar * ch = parag->at( cursor->index() );
            undoRedoInfo.text += ch->c;
            copyCharFormatting( parag, cursor->index(), undoRedoInfo.text.length()-1, true );
        }
        KoParagLayout paragLayout;
        if ( parag->next() )
            paragLayout = parag->next()->paragLayout();

        KoTextParag *old = cursor->parag();
        if ( cursor->remove() ) {
            if ( old != cursor->parag() && m_lastFormatted == old ) // 'old' has been deleted
                m_lastFormatted = cursor->parag() ? cursor->parag()->prev() : 0;
            undoRedoInfo.text += "\n";
            undoRedoInfo.oldParagLayouts << paragLayout;
        } else
            emit paragraphModified( old, RemoveChar, cursor->index(), 1 );
    } break;
    case ActionBackspace: {
        // Remove counter
        if ( parag->counter() && parag->counter()->style() != KoParagCounter::STYLE_NONE && cursor->index() == 0 ) {
            // parag->decDepth(); // We don't have support for nested lists at the moment
                                  // (only in titles, but you don't want Backspace to move it up)
            KoParagCounter c;
            c.setDepth( parag->counter()->depth() );
            KCommand *cmd=setCounterCommand( cursor, c );
            if(cmd)
                emit newCommand(cmd);
        }
        else if ( !cursor->atParagStart() )
        {
            checkUndoRedoInfo( cursor, UndoRedoInfo::Delete );
            if ( !undoRedoInfo.valid() ) {
                newPlaceHolderCommand( i18n("Delete Text") );
                undoRedoInfo.id = parag->paragId();
                undoRedoInfo.index = cursor->index();
                undoRedoInfo.text = QString::null;
                undoRedoInfo.oldParagLayouts << parag->paragLayout();
            }
            undoRedoInfo.text.insert( 0, cursor->parag()->at( cursor->index()-1 ) );
            copyCharFormatting( cursor->parag(), cursor->index()-1, 0, true );
            undoRedoInfo.index = cursor->index()-1;
            //KoParagLayout paragLayout = cursor->parag()->paragLayout();
            cursor->removePreviousChar();
            emit paragraphModified( cursor->parag(), RemoveChar, cursor->index(),1 );
            m_lastFormatted = cursor->parag();
        } else if ( parag->prev() ) { // joining paragraphs
            emit paragraphDeleted( cursor->parag() );
            clearUndoRedoInfo();
            textdoc->setSelectionStart( KoTextDocument::Temp, cursor );
            cursor->gotoPreviousLetter();
            textdoc->setSelectionEnd( KoTextDocument::Temp, cursor );
            removeSelectedText( cursor, KoTextDocument::Temp, i18n( "Delete Text" ) );
            emit paragraphModified( cursor->parag(), AddChar, cursor->index(), cursor->parag()->length() - cursor->index() );
        }
    } break;
    case ActionReturn: {
        checkUndoRedoInfo( cursor, UndoRedoInfo::Return );
        if ( !undoRedoInfo.valid() ) {
            newPlaceHolderCommand( i18n("Insert Text") );
            undoRedoInfo.id = cursor->parag()->paragId();
            undoRedoInfo.index = cursor->index();
            undoRedoInfo.text = QString::null;
        }
        undoRedoInfo.text += "\n";
        if ( cursor->parag() )
        {
                QString last_line = cursor->parag()->toString();
                last_line.remove(0,last_line.find(' ')+1);

                if( last_line.isEmpty() && cursor->parag()->counter() && cursor->parag()->counter()->numbering() == KoParagCounter::NUM_LIST ) //if the previous line the in paragraph is empty
                {
                        KoParagCounter c;
                        KCommand *cmd=setCounterCommand( cursor, c );
                        if(cmd)
                                emit newCommand(cmd);
                        setLastFormattedParag( cursor->parag() );
                        cursor->parag()->setNoCounter();

                        formatMore( 2 );
                        emit repaintChanged( this );
                        emit ensureCursorVisible();
                        emit showCursor();
                        emit updateUI( doUpdateCurrentFormat );
                        return;
                }
                else
                        cursor->splitAndInsertEmptyParag();
        }

        Q_ASSERT( cursor->parag()->prev() );
        setLastFormattedParag( cursor->parag() );

        doUpdateCurrentFormat = false;
        KoParagStyle * style = cursor->parag()->prev()->style();
        if ( style )
        {
            KoParagStyle * newStyle = style->followingStyle();
            if ( newStyle && style != newStyle ) // different "following style" applied
            {
                doUpdateCurrentFormat = true;
                //currentFormat = textdoc->formatCollection()->format( cursor->parag()->paragFormat() );
                //kDebug(32500) << "KoTextFrameSet::doKeyboardAction currentFormat=" << currentFormat << " " << currentFormat->key() << endl;
            }
        }
        if ( cursor->parag()->joinBorder() && cursor->parag()->bottomBorder().width() > 0 )
            cursor->parag()->prev()->setChanged( true );
        if ( cursor->parag()->joinBorder() && cursor->parag()->next() && cursor->parag()->next()->joinBorder() && cursor->parag()->bottomBorder() == cursor->parag()->next()->bottomBorder())
            cursor->parag()->next()->setChanged( true );
        emit paragraphCreated( cursor->parag() );

    } break;
    case ActionKill:
        // Nothing to kill if at end of very last paragraph
        if ( !cursor->atParagEnd() || cursor->parag()->next() ) {
            checkUndoRedoInfo( cursor, UndoRedoInfo::Delete );
            if ( !undoRedoInfo.valid() ) {
                newPlaceHolderCommand( i18n("Delete Text") );
                undoRedoInfo.id = cursor->parag()->paragId();
                undoRedoInfo.index = cursor->index();
                undoRedoInfo.text = QString::null;
                undoRedoInfo.oldParagLayouts << parag->paragLayout();
            }
            if ( cursor->atParagEnd() ) {
                // Get paraglayout from next parag (next can't be 0 here)
                KoParagLayout paragLayout = parag->next()->paragLayout();
                if ( cursor->remove() )
                {
                    m_lastFormatted = cursor->parag();
                    undoRedoInfo.text += "\n";
                    undoRedoInfo.oldParagLayouts << paragLayout;
                }
            } else {
                int oldLen = undoRedoInfo.text.length();
                undoRedoInfo.text += cursor->parag()->string()->toString().mid( cursor->index() );
                for ( int i = cursor->index(); i < cursor->parag()->length(); ++i )
                    copyCharFormatting( cursor->parag(), i, oldLen + i - cursor->index(), true );
                cursor->killLine();
                emit paragraphModified( cursor->parag(), RemoveChar, cursor->index(), cursor->parag()->length()-cursor->index() );
            }
        }
        break;
    }

    if ( !undoRedoInfo.customItemsMap.isEmpty() )
        clearUndoRedoInfo();

    formatMore( 2 );
    emit repaintChanged( this );
    emit ensureCursorVisible();
    emit showCursor();
    emit updateUI( doUpdateCurrentFormat );
}

void KoTextObject::insert( KoTextCursor * cursor, KoTextFormat * currentFormat,
                           const QString &txt, const QString & commandName, KoTextDocument::SelectionId selectionId,
                           int insertFlags, CustomItemsMap customItemsMap )
{
    if ( protectContent() )
        return;
    const bool checkNewLine = insertFlags & CheckNewLine;
    const bool removeSelected = ( insertFlags & DoNotRemoveSelected ) == 0;
    const bool repaint = ( insertFlags & DoNotRepaint ) == 0;
    //kDebug(32500) << "KoTextObject::insert txt=" << txt << endl;
    bool tinyRepaint = !checkNewLine;
    if ( repaint )
        emit hideCursor();
    if ( textdoc->hasSelection( selectionId, true ) && removeSelected ) {
        kDebug() << k_funcinfo << "removing selection " << selectionId << endl;
        // call replaceSelectionCommand, which will call insert() back, but this time without a selection.
        emitNewCommand(replaceSelectionCommand( cursor, txt, commandName, selectionId, insertFlags, customItemsMap ));
        return;
    }
    // Now implement overwrite mode, similarly.
    if ( insertFlags & OverwriteMode ) {
        textdoc->setSelectionStart( KoTextDocument::Temp, cursor );
        KoTextCursor oc = *cursor;
        kDebug(32500) << "overwrite: going to insert " << txt.length() << " chars; idx=" << oc.index() << endl;
        oc.setIndex( qMin( oc.index() + (int)txt.length(), oc.parag()->lastCharPos() + 1 ) );
        kDebug(32500) << "overwrite: removing from " << cursor->index() << " to " << oc.index() << endl;
        if ( oc.index() > cursor->index() )
        {
            textdoc->setSelectionEnd( KoTextDocument::Temp, &oc );
            int newInsertFlags = insertFlags & ~OverwriteMode;
            newInsertFlags &= ~DoNotRemoveSelected;
            emitNewCommand(replaceSelectionCommand( cursor, txt, commandName, KoTextDocument::Temp, newInsertFlags, customItemsMap ));
            return;
        }
    }
    KoTextCursor c2 = *cursor;
    // Make everything ready for undo/redo (new command, or add to current one)
    if ( !customItemsMap.isEmpty() )
        clearUndoRedoInfo();
    checkUndoRedoInfo( cursor, UndoRedoInfo::Insert );
    if ( !undoRedoInfo.valid() ) {
        if ( !commandName.isNull() ) // see replace-selection
            newPlaceHolderCommand( commandName );
        undoRedoInfo.id = cursor->parag()->paragId();
        undoRedoInfo.index = cursor->index();
        undoRedoInfo.text = QString::null;
    }
    int oldLen = undoRedoInfo.text.length();
    KoTextCursor oldCursor = *cursor;
    bool wasChanged = cursor->parag()->hasChanged();
    int origLine; // the line the cursor was on before the insert
    oldCursor.parag()->lineStartOfChar( oldCursor.index(), 0, &origLine );

    // insert the text - finally!
    cursor->insert( txt, checkNewLine );

    setLastFormattedParag( checkNewLine ? oldCursor.parag() : cursor->parag() );

    if ( !customItemsMap.isEmpty() ) {
        customItemsMap.insertItems( oldCursor, txt.length() );
        undoRedoInfo.customItemsMap = customItemsMap;
        tinyRepaint = false;
    }

    textdoc->setSelectionStart( KoTextDocument::Temp, &oldCursor );
    textdoc->setSelectionEnd( KoTextDocument::Temp, cursor );
    //kDebug(32500) << "KoTextObject::insert setting format " << currentFormat << endl;
    textdoc->setFormat( KoTextDocument::Temp, currentFormat, KoTextFormat::Format );
    textdoc->setFormat( KoTextDocument::InputMethodPreedit, currentFormat, KoTextFormat::Format );
    textdoc->removeSelection( KoTextDocument::Temp );

    if ( !customItemsMap.isEmpty() ) {
        // Some custom items (e.g. variables) depend on the format
        CustomItemsMap::Iterator it = customItemsMap.begin();
        for ( ; it != customItemsMap.end(); ++it )
            it.data()->resize();
    }

    // Speed optimization: if we only type a char, and it doesn't
    // invalidate the next parag, only format the current one
    // ### This idea is wrong. E.g. when the last parag grows and must create a new page.
#if 0
    KoTextParag *parag = cursor->parag();
    if ( !checkNewLine && m_lastFormatted == parag && ( !parag->next() || parag->next()->isValid() ) )
    {
        parag->format();
        m_lastFormatted = m_lastFormatted->next();
    }
#endif
    // Call formatMore until necessary. This will create new pages if needed.
    // Doing this here means callers don't have to do it, and cursor can be positionned correctly.
    ensureFormatted( cursor->parag() );

    // Speed optimization: if we only type a char, only repaint from current line
    // (In fact the one before. When typing a space, a word could move up to the
    // line before, we need to repaint it too...)
    if ( !checkNewLine && tinyRepaint && !wasChanged )
    {
        // insert() called format() which called setChanged().
        // We're reverting that, and calling setLineChanged() only.
        Q_ASSERT( cursor->parag() == oldCursor.parag() ); // no newline!
        KoTextParag* parag = cursor->parag();
        // If the new char led to a new line,
        // the wordwrap could have changed on the line above
        // This is why we use origLine and not calling lineStartOfChar here.
        parag->setChanged( false );
        parag->setLineChanged( origLine - 1 ); // if origLine=0, it'll pass -1, which is 'all'
    }

    if ( repaint ) {
        emit repaintChanged( this );
        emit ensureCursorVisible();
        emit showCursor();
        // we typed the first char of a paragraph in AlignAuto mode -> show correct alignment in UI
        if ( oldCursor.index() == 0 && oldCursor.parag()->alignment() == Qt::AlignLeft )
            emit updateUI( true );

    }
    undoRedoInfo.text += txt;
    for ( int i = 0; i < (int)txt.length(); ++i ) {
        if ( txt[ oldLen + i ] != '\n' )
            copyCharFormatting( c2.parag(), c2.index(), oldLen + i, false );
        c2.gotoNextLetter();
    }

    if ( !removeSelected ) {
        // ## not sure why we do this. I'd prefer leaving the selection unchanged...
        // but then it'd need adjustements in the offsets etc.
        if ( textdoc->removeSelection( selectionId ) && repaint )
            selectionChangedNotify(); // does the repaint
    }
    if ( !customItemsMap.isEmpty() ) {
        clearUndoRedoInfo();
    }

    // Notifications
    emit paragraphModified( oldCursor.parag(), AddChar, cursor->index(), txt.length() );
    if (checkNewLine) {
        KoTextParag* p = oldCursor.parag()->next();
        while ( p && p != cursor->parag() ) {
            emit paragraphCreated( p );
            p = p->next();
        }
    }
}

void KoTextObject::pasteText( KoTextCursor * cursor, const QString & text, KoTextFormat * currentFormat, bool removeSelected )
{
    if ( protectContent() )
        return;
    kDebug(32500) << "KoTextObject::pasteText cursor parag=" << cursor->parag()->paragId() << endl;
    QString t = text;
    // Need to convert CRLF to NL
    QRegExp crlf( QString::fromLatin1("\r\n") );
    t.replace( crlf, QChar('\n') );
    // Convert non-printable chars
    for ( int i=0; (uint) i<t.length(); i++ ) {
        if ( t[ i ] < ' ' && t[ i ] != '\n' && t[ i ] != '\t' )
            t[ i ] = ' ';
    }
    if ( !t.isEmpty() )
    {
        int insertFlags = CheckNewLine;
        if ( !removeSelected )
            insertFlags |= DoNotRemoveSelected;
        insert( cursor, currentFormat, t, i18n("Paste Text"),
                KoTextDocument::Standard, insertFlags );
        formatMore( 2 );
        emit repaintChanged( this );
    }
}

KCommand* KoTextObject::setParagLayoutCommand( KoTextCursor * cursor, const KoParagLayout& paragLayout,
                                               KoTextDocument::SelectionId selectionId, int paragLayoutFlags,
                                               int marginIndex, bool createUndoRedo )
{
    if ( protectContent() )
        return 0;
    storeParagUndoRedoInfo( cursor, selectionId );
    undoRedoInfo.type = UndoRedoInfo::Invalid; // tricky, we don't want clear() to create a command
    if ( paragLayoutFlags != 0 )
    {
        emit hideCursor();
        if ( !textdoc->hasSelection( selectionId, true ) ) {
            cursor->parag()->setParagLayout( paragLayout, paragLayoutFlags, marginIndex );
            setLastFormattedParag( cursor->parag() );
        } else {
            KoTextParag *start = textdoc->selectionStart( selectionId );
            KoTextParag *end = textdoc->selectionEnd( selectionId );
            for ( ; start && start != end->next() ; start = start->next() ) {
                if ( paragLayoutFlags == KoParagLayout::BulletNumber && start->length() <= 1 )
                    continue; // don't apply to empty paragraphs (#25742, #34062)
                start->setParagLayout( paragLayout, paragLayoutFlags, marginIndex );
            }
            setLastFormattedParag( start );
        }

        formatMore( 2 );
        emit repaintChanged( this );
        emit showCursor();
        emit updateUI( true );

        if ( createUndoRedo )
        {
            //kDebug(32500) << "KoTextObject::applyStyle KoTextParagCommand" << endl;
            KoTextDocCommand * cmd = new KoTextParagCommand( textdoc, undoRedoInfo.id, undoRedoInfo.eid,
                                                             undoRedoInfo.oldParagLayouts,
                                                             paragLayout, paragLayoutFlags,
                                                             (Q3StyleSheetItem::Margin)marginIndex );
            textdoc->addCommand( cmd );
            return new KoTextCommand( this, /*cmd, */"related to KoTextParagCommand" );
        }
    }
    return 0;
}


void KoTextObject::applyStyle( KoTextCursor * cursor, const KoParagStyle * newStyle,
                               KoTextDocument::SelectionId selectionId,
                               int paragLayoutFlags, int formatFlags,
                               bool createUndoRedo, bool interactive )
{
    KCommand *cmd = applyStyleCommand( cursor, newStyle, selectionId,
                                       paragLayoutFlags, formatFlags,
                                       createUndoRedo, interactive );
    if ( createUndoRedo && cmd )
        emit newCommand( cmd );
    else
        Q_ASSERT( !cmd ); // mem leak, if applyStyleCommand created a command despite createUndoRedo==false!
}

KCommand *KoTextObject::applyStyleCommand( KoTextCursor * cursor, const KoParagStyle * newStyle,
                               KoTextDocument::SelectionId selectionId,
                               int paragLayoutFlags, int formatFlags,
                               bool createUndoRedo, bool interactive )
{
    if ( protectContent())
        return 0L;
    if ( interactive )
        emit hideCursor();
    if ( !textdoc->hasSelection( selectionId, true ) && !cursor)
        return 0L;
    /// Applying a style is three distinct operations :
    /// 1 - Changing the paragraph settings (setParagLayout)
    /// 2 - Changing the character formatting for each char in the paragraph (setFormat(indices))
    /// 3 - Changing the character formatting for the whole paragraph (setFormat()) [just in case]
    /// -> We need a macro command to hold the 3 commands
    KMacroCommand * macroCmd = createUndoRedo ? new KMacroCommand( i18n("Apply Style %1").
                                                                   arg(newStyle->displayName() ) ) : 0;

    // 1
    //kDebug(32500) << "KoTextObject::applyStyle setParagLayout" << endl;
    KCommand* cmd = setParagLayoutCommand( cursor, newStyle->paragLayout(), selectionId, paragLayoutFlags, -1, createUndoRedo );
    if ( cmd )
        macroCmd->addCommand( cmd );

    // 2
    //kDebug(32500) << "KoTextObject::applyStyle gathering text and formatting" << endl;
    KoTextParag * firstParag;
    KoTextParag * lastParag;
    if ( !textdoc->hasSelection( selectionId, true ) ) {
        // No selection -> apply style formatting to the whole paragraph
        firstParag = cursor->parag();
        lastParag = cursor->parag();
    }
    else
    {
        firstParag = textdoc->selectionStart( selectionId );
        lastParag = textdoc->selectionEnd( selectionId );
    }

    if ( formatFlags != 0 )
    {
        KoTextFormat * newFormat = textdoc->formatCollection()->format( &newStyle->format() );

        if ( createUndoRedo )
        {
            Q3ValueList<KoTextFormat *> lstFormats;
            //QString str;
            for ( KoTextParag * parag = firstParag ; parag && parag != lastParag->next() ; parag = parag->next() )
            {
                //str += parag->string()->toString() + '\n';
                lstFormats.append( parag->paragFormat() );
            }
            KoTextCursor c1( textdoc );
            c1.setParag( firstParag );
            c1.setIndex( 0 );
            KoTextCursor c2( textdoc );
            c2.setParag( lastParag );
            c2.setIndex( lastParag->string()->length() );
            undoRedoInfo.clear();
            undoRedoInfo.type = UndoRedoInfo::Invalid; // same trick
            readFormats( c1, c2 ); // gather char-format info but not paraglayouts nor customitems

            KoTextDocCommand * cmd = new KoTextFormatCommand( textdoc, firstParag->paragId(), 0,
                                                         lastParag->paragId(), c2.index(),
                                                         undoRedoInfo.text.rawData(), newFormat,
                                                         formatFlags );
            textdoc->addCommand( cmd );
            macroCmd->addCommand( new KoTextCommand( this, /*cmd, */"related to KoTextFormatCommand" ) );

            // sub-command for '3' (paragFormat)
            cmd = new KoParagFormatCommand( textdoc, firstParag->paragId(), lastParag->paragId(),
                                            lstFormats, newFormat );
            textdoc->addCommand( cmd );
            macroCmd->addCommand( new KoTextCommand( this, /*cmd, */"related to KoParagFormatCommand" ) );
        }

        // apply '2' and '3' (format)
        for ( KoTextParag * parag = firstParag ; parag && parag != lastParag->next() ; parag = parag->next() )
        {
            //kDebug(32500) << "KoTextObject::applyStyle parag:" << parag->paragId()
            //               << ", from 0 to " << parag->string()->length() << ", format=" << newFormat << endl;
            parag->setFormat( 0, parag->string()->length(), newFormat, true, formatFlags );
            parag->setFormat( newFormat );
        }
        //currentFormat = textdoc->formatCollection()->format( newFormat );
        //kDebug(32500) << "KoTextObject::applyStyle currentFormat=" << currentFormat << " " << currentFormat->key() << endl;
    }

    //resize all variables after applying the style
    Q3PtrListIterator<KoTextCustomItem> cit( textdoc->allCustomItems() );
    for ( ; cit.current() ; ++cit )
        cit.current()->resize();


    if ( interactive )
    {
        setLastFormattedParag( firstParag );
        formatMore( 2 );
        emit repaintChanged( this );
        emit updateUI( true );
        emit showCursor();
    }

    undoRedoInfo.clear();

    return macroCmd;
}

void KoTextObject::applyStyleChange( KoStyleChangeDefMap changed )
{
#if 0 //#ifndef NDEBUG
    kDebug(32500) << "KoTextObject::applyStyleChange " << changed.count() << " styles." << endl;
    for( KoStyleChangeDefMap::const_iterator it = changed.begin(); it != changed.end(); ++it ) {
        kDebug(32500) << " " << it.key()->name()
                       << " paragLayoutChanged=" << (*it).paragLayoutChanged
                       << " formatChanged=" << (*it).formatChanged
                       << endl;
    }
#endif

    KoTextParag *p = textdoc->firstParag();
    while ( p ) {
        KoStyleChangeDefMap::Iterator it = changed.find( p->style() );
        if ( it != changed.end() )
        {
            if ( (*it).paragLayoutChanged == -1 || (*it).formatChanged == -1 ) // Style has been deleted
            {
                p->setStyle( m_defaultStyle ); // keeps current formatting
                // TODO, make this undoable somehow
            }
            else
            {
                // Apply this style again, to get the changes
                KoTextCursor cursor( textdoc );
                cursor.setParag( p );
                cursor.setIndex( 0 );
                //kDebug(32500) << "KoTextObject::applyStyleChange applying to paragraph " << p << " " << p->paragId() << endl;
                applyStyle( &cursor, it.key(),
                            KoTextDocument::Temp, // A selection we can't have at this point
                            (*it).paragLayoutChanged, (*it).formatChanged,
                            false, false ); // don't create undo/redo, not interactive
            }
        } else {
            //kDebug(32500) << "KoTextObject::applyStyleChange leaving paragraph unchanged: " << p << " " << p->paragId() << endl;
        }

        p = p->next();
    }
    setLastFormattedParag( textdoc->firstParag() );
    formatMore( 2 );
    emit repaintChanged( this );
    emit updateUI( true );
}

/** Implementation of setFormatCommand from KoTextFormatInterface - apply change to the whole document */
KCommand *KoTextObject::setFormatCommand( const KoTextFormat *format, int flags, bool zoomFont )
{
    textdoc->selectAll( KoTextDocument::Temp );
    KCommand *cmd = setFormatCommand( 0L, 0L, format, flags, zoomFont, KoTextDocument::Temp );
    textdoc->removeSelection( KoTextDocument::Temp );
    return cmd;
}

KCommand * KoTextObject::setFormatCommand( KoTextCursor * cursor, KoTextFormat ** pCurrentFormat, const KoTextFormat *format, int flags, bool /*zoomFont*/, KoTextDocument::SelectionId selectionId )
{
    KCommand *ret = 0;
    if ( protectContent() )
        return ret;

    KoTextFormat* newFormat = 0;
    // Get new format from collection if
    // - caller has notion of a "current format" and new format is different
    // - caller has no notion of "current format", e.g. whole textobjects
    bool isNewFormat = ( pCurrentFormat && *pCurrentFormat && (*pCurrentFormat)->key() != format->key() );
    if ( isNewFormat || !pCurrentFormat )
    {
#if 0
        int origFontSize = 0;
        if ( zoomFont ) // The format has a user-specified font (e.g. setting a style, or a new font size)
        {
            origFontSize = format->pointSize();
            format->setPointSize( zoomedFontSize( origFontSize ) );
            //kDebug(32500) << "KoTextObject::setFormatCommand format " << format->key() << " zoomed from " << origFontSize << " to " << format->font().pointSizeFloat() << endl;
        }
#endif
        // Remove ref to current format, if caller wanted that
        if ( pCurrentFormat )
            (*pCurrentFormat)->removeRef();
        // Find format in collection
        newFormat = textdoc->formatCollection()->format( format );
        if ( newFormat->isMisspelled() ) {
            KoTextFormat fNoMisspelled( *newFormat );
            newFormat->removeRef();
            fNoMisspelled.setMisspelled( false );
            newFormat = textdoc->formatCollection()->format( &fNoMisspelled );
        }
        if ( pCurrentFormat )
            (*pCurrentFormat) = newFormat;
    }

    if ( textdoc->hasSelection( selectionId, true ) ) {
        emit hideCursor();
        KoTextCursor c1 = textdoc->selectionStartCursor( selectionId );
        KoTextCursor c2 = textdoc->selectionEndCursor( selectionId );
        undoRedoInfo.clear();
        int id = c1.parag()->paragId();
        int index = c1.index();
        int eid = c2.parag()->paragId();
        int eindex = c2.index();
        readFormats( c1, c2 ); // read previous formatting info
        //kDebug(32500) << "KoTextObject::setFormatCommand undoredo info done" << endl;
        textdoc->setFormat( selectionId, format, flags );
        if ( !undoRedoInfo.customItemsMap.isEmpty() )
        {
            // Some custom items (e.g. variables) depend on the format
            CustomItemsMap::Iterator it = undoRedoInfo.customItemsMap.begin();
            for ( ; it != undoRedoInfo.customItemsMap.end(); ++it )
                it.data()->resize();
        }
        KoTextFormatCommand *cmd = new KoTextFormatCommand(
            textdoc, id, index, eid, eindex, undoRedoInfo.text.rawData(),
            format, flags );
        textdoc->addCommand( cmd );
        ret = new KoTextCommand( this, /*cmd, */i18n("Format Text") );
        undoRedoInfo.clear();
        setLastFormattedParag( c1.parag() );
        formatMore( 2 );
        emit repaintChanged( this );
        emit showCursor();
    }
    if ( isNewFormat ) {
        emit showCurrentFormat();
        //kDebug(32500) << "KoTextObject::setFormatCommand index=" << cursor->index() << " length-1=" << cursor->parag()->length() - 1 << endl;
        if ( cursor && cursor->index() == cursor->parag()->length() - 1 ) {
            newFormat->addRef();
            cursor->parag()->string()->setFormat( cursor->index(), newFormat, TRUE );
            if ( cursor->parag()->length() == 1 ) {
                newFormat->addRef();
                cursor->parag()->setFormat( newFormat );
                cursor->parag()->invalidate(0);
                cursor->parag()->format();
                emit repaintChanged( this );
            }
        }
    }
    return ret;
}

void KoTextObject::setFormat( KoTextCursor * cursor, KoTextFormat ** currentFormat, KoTextFormat *format, int flags, bool zoomFont )
{
    if ( protectContent() )
        return;
    KCommand *cmd = setFormatCommand( cursor, currentFormat, format, flags, zoomFont );
    if (cmd)
        emit newCommand( cmd );
}

void KoTextObject::emitNewCommand(KCommand *cmd)
{
    if(cmd)
        emit newCommand( cmd );
}

KCommand *KoTextObject::setCounterCommand( KoTextCursor * cursor, const KoParagCounter & counter, KoTextDocument::SelectionId selectionId  )
{
    if ( protectContent() )
        return 0L;
    const KoParagCounter * curCounter = 0L;
    if(cursor)
        curCounter=cursor->parag()->counter();
    if ( !textdoc->hasSelection( selectionId, true ) &&
         curCounter && counter == *curCounter ) {
        return 0L;
    }
    emit hideCursor();
    storeParagUndoRedoInfo( cursor, selectionId );
    if ( !textdoc->hasSelection( selectionId, true ) && cursor) {
        cursor->parag()->setCounter( counter );
        setLastFormattedParag( cursor->parag() );
    } else {
        KoTextParag *start = textdoc->selectionStart( selectionId );
        KoTextParag *end = textdoc->selectionEnd( selectionId );
#if 0
        // Special hack for BR25742, don't apply bullet to last empty parag of the selection
        if ( start != end && end->length() <= 1 )
        {
            end = end->prev();
            undoRedoInfo.eid = end->paragId();
        }
#endif
        setLastFormattedParag( start );
        for ( ; start && start != end->next() ; start = start->next() )
        {
            if ( start->length() > 1 )  // don't apply to empty paragraphs (#25742, #34062)
                start->setCounter( counter );
        }
    }
    formatMore( 2 );
    emit repaintChanged( this );
    if ( !undoRedoInfo.newParagLayout.counter )
        undoRedoInfo.newParagLayout.counter = new KoParagCounter;
    *undoRedoInfo.newParagLayout.counter = counter;
    KoTextParagCommand *cmd = new KoTextParagCommand(
        textdoc, undoRedoInfo.id, undoRedoInfo.eid,
        undoRedoInfo.oldParagLayouts, undoRedoInfo.newParagLayout,
        KoParagLayout::BulletNumber );
    textdoc->addCommand( cmd );

    undoRedoInfo.clear(); // type is still Invalid -> no command created
    emit showCursor();
    emit updateUI( true );
    return new KoTextCommand( this, /*cmd, */i18n("Change List Type") );
}

KCommand * KoTextObject::setAlignCommand( KoTextCursor * cursor, int align, KoTextDocument::SelectionId selectionId )
{
    if ( protectContent() )
        return 0L;
    if ( !textdoc->hasSelection( selectionId, true ) && cursor &&
         (int)cursor->parag()->alignment() == align )
        return 0L; // No change needed.

    emit hideCursor();
    storeParagUndoRedoInfo( cursor ,selectionId );
    if ( !textdoc->hasSelection( selectionId, true ) &&cursor ) {
        cursor->parag()->setAlign(align);
        setLastFormattedParag( cursor->parag() );
    }
    else
    {
        KoTextParag *start = textdoc->selectionStart( selectionId );
        KoTextParag *end = textdoc->selectionEnd( selectionId  );
        setLastFormattedParag( start );
        for ( ; start && start != end->next() ; start = start->next() )
            start->setAlign(align);
    }
    formatMore( 2 );
    emit repaintChanged( this );
    undoRedoInfo.newParagLayout.alignment = align;
    KoTextParagCommand *cmd = new KoTextParagCommand(
        textdoc, undoRedoInfo.id, undoRedoInfo.eid,
        undoRedoInfo.oldParagLayouts, undoRedoInfo.newParagLayout,
        KoParagLayout::Alignment );
    textdoc->addCommand( cmd );
    undoRedoInfo.clear(); // type is still Invalid -> no command created
    emit showCursor();
    emit updateUI( true );
    return new KoTextCommand( this, /*cmd, */i18n("Change Alignment") );
}

KCommand * KoTextObject::setMarginCommand( KoTextCursor * cursor, Q3StyleSheetItem::Margin m, double margin , KoTextDocument::SelectionId selectionId ) {
    if ( protectContent() )
        return 0L;

    //kDebug(32500) << "KoTextObject::setMargin " << m << " to value " << margin << endl;
    //kDebug(32500) << "Current margin is " << cursor->parag()->margin(m) << endl;
    if ( !textdoc->hasSelection( selectionId, true ) && cursor &&
         cursor->parag()->margin(m) == margin )
        return 0L; // No change needed.

    emit hideCursor();
    storeParagUndoRedoInfo( cursor, selectionId );
    if ( !textdoc->hasSelection( selectionId, true )&&cursor ) {
        cursor->parag()->setMargin(m, margin);
        setLastFormattedParag( cursor->parag() );
    }
    else
    {
        KoTextParag *start = textdoc->selectionStart( selectionId );
        KoTextParag *end = textdoc->selectionEnd( selectionId );
        setLastFormattedParag( start );
        for ( ; start && start != end->next() ; start = start->next() )
            start->setMargin(m, margin);
    }
    formatMore( 2 );
    emit repaintChanged( this );
    undoRedoInfo.newParagLayout.margins[m] = margin;
    KoTextParagCommand *cmd = new KoTextParagCommand(
        textdoc, undoRedoInfo.id, undoRedoInfo.eid,
        undoRedoInfo.oldParagLayouts, undoRedoInfo.newParagLayout,
        KoParagLayout::Margins, m );
    textdoc->addCommand( cmd );
    QString name;
    if ( m == Q3StyleSheetItem::MarginFirstLine )
        name = i18n("Change First Line Indent");
    else if ( m == Q3StyleSheetItem::MarginLeft || m == Q3StyleSheetItem::MarginRight )
        name = i18n("Change Indent");
    else
        name = i18n("Change Paragraph Spacing");
    undoRedoInfo.clear();
    emit showCursor();
    emit updateUI( true );
    return  new KoTextCommand( this, /*cmd, */name );
}

KCommand * KoTextObject::setBackgroundColorCommand( KoTextCursor * cursor,
                                                    const QColor & color,
                                                    KoTextDocument::SelectionId selectionId ) {
    if ( protectContent() )
        return 0L;

    if ( !textdoc->hasSelection( selectionId, true ) && cursor &&
         cursor->parag()->backgroundColor() == color )
        return 0L; // No change needed.

    emit hideCursor();
    storeParagUndoRedoInfo( cursor, selectionId );
    if ( !textdoc->hasSelection( selectionId, true )&&cursor )
    {
        // Update a single paragraph
        cursor->parag()->setBackgroundColor(color);
        setLastFormattedParag( cursor->parag() );
    }
    else
    {
        // Update multiple paragraphs
        KoTextParag *start = textdoc->selectionStart( selectionId );
        KoTextParag *end = textdoc->selectionEnd( selectionId );
        setLastFormattedParag( start );
        for ( ; start && start != end->next() ; start = start->next() )
            start->setBackgroundColor(color);
    }
    formatMore( 2 );
    emit repaintChanged( this );

    // Update undo/redo info
    undoRedoInfo.newParagLayout.backgroundColor = color;
    KoTextParagCommand *cmd = new KoTextParagCommand(
        textdoc, undoRedoInfo.id, undoRedoInfo.eid,
        undoRedoInfo.oldParagLayouts, undoRedoInfo.newParagLayout,
        KoParagLayout::BackgroundColor );
    textdoc->addCommand( cmd );
    undoRedoInfo.clear();

    emit showCursor();
    emit updateUI( true );
    return new KoTextCommand( this, /*cmd, */ i18n("Change Paragraph Background Color" ) );
}

KCommand * KoTextObject::setLineSpacingCommand( KoTextCursor * cursor, double spacing, KoParagLayout::SpacingType _type, KoTextDocument::SelectionId selectionId )
{
    if ( protectContent() )
        return 0L;
    //kDebug(32500) << "KoTextObject::setLineSpacing to value " << spacing << endl;
    //kDebug(32500) << "Current spacing is " << cursor->parag()->kwLineSpacing() << endl;
    //kDebug(32500) << "Comparison says " << ( cursor->parag()->kwLineSpacing() == spacing ) << endl;
    //kDebug(32500) << "hasSelection " << textdoc->hasSelection( selectionId ) << endl;
    if ( !textdoc->hasSelection( selectionId, true ) && cursor &&
         cursor->parag()->kwLineSpacing() == spacing
        && cursor->parag()->kwLineSpacingType() == _type)
        return 0L; // No change needed.

    emit hideCursor();
    storeParagUndoRedoInfo( cursor, selectionId );
    if ( !textdoc->hasSelection( selectionId, true ) && cursor ) {
        cursor->parag()->setLineSpacing(spacing);
        cursor->parag()->setLineSpacingType( _type);
        setLastFormattedParag( cursor->parag() );
    }
    else
    {
        KoTextParag *start = textdoc->selectionStart( selectionId );
        KoTextParag *end = textdoc->selectionEnd( selectionId );
        setLastFormattedParag( start );
        for ( ; start && start != end->next() ; start = start->next() )
        {
            start->setLineSpacing(spacing);
            start->setLineSpacingType( _type);
        }
    }
    formatMore( 2 );
    emit repaintChanged( this );
    undoRedoInfo.newParagLayout.setLineSpacingValue( spacing );
    undoRedoInfo.newParagLayout.lineSpacingType = _type;
    KoTextParagCommand *cmd = new KoTextParagCommand(
        textdoc, undoRedoInfo.id, undoRedoInfo.eid,
        undoRedoInfo.oldParagLayouts, undoRedoInfo.newParagLayout,
        KoParagLayout::LineSpacing );
    textdoc->addCommand( cmd );

    undoRedoInfo.clear();
    emit showCursor();
    return new KoTextCommand( this, /*cmd, */i18n("Change Line Spacing") );
}


KCommand * KoTextObject::setBordersCommand( KoTextCursor * cursor, const KoBorder& leftBorder, const KoBorder& rightBorder, const KoBorder& topBorder, const KoBorder& bottomBorder , KoTextDocument::SelectionId selectionId )
{
    if ( protectContent() )
        return 0L;
  if ( !textdoc->hasSelection( selectionId, true ) && cursor &&
       cursor->parag()->leftBorder() ==leftBorder &&
       cursor->parag()->rightBorder() ==rightBorder &&
       cursor->parag()->topBorder() ==topBorder &&
       cursor->parag()->bottomBorder() ==bottomBorder )
        return 0L; // No change needed.

    emit hideCursor();
    bool borderOutline = false;
    storeParagUndoRedoInfo( cursor, selectionId );
    if ( !textdoc->hasSelection( selectionId, true ) ) {
      cursor->parag()->setLeftBorder(leftBorder);
      cursor->parag()->setRightBorder(rightBorder);
      cursor->parag()->setBottomBorder(bottomBorder);
      cursor->parag()->setTopBorder(topBorder);
      setLastFormattedParag( cursor->parag() );

      if ( cursor->parag()->next() )
        cursor->parag()->next()->setChanged( true );
       if ( cursor->parag()->prev() )
        cursor->parag()->prev()->setChanged( true );
    }
    else
    {
        KoTextParag *start = textdoc->selectionStart( selectionId );
        KoTextParag *end = textdoc->selectionEnd( selectionId );
        setLastFormattedParag( start );
        KoBorder tmpBorder;
        tmpBorder.setPenWidth(0);
        for ( ; start && start != end->next() ; start = start->next() )
          {
            start->setLeftBorder(leftBorder);
            start->setRightBorder(rightBorder);
            //remove border
            start->setTopBorder(tmpBorder);
            start->setBottomBorder(tmpBorder);
          }
        end->setBottomBorder(bottomBorder);
        textdoc->selectionStart( selectionId )->setTopBorder(topBorder);
        borderOutline = true;

        if ( start && start->prev() )
            start->prev()->setChanged( true );
        if ( end && end->next() )
            end->next()->setChanged( true );
    }
    formatMore( 2 );
    emit repaintChanged( this );
    undoRedoInfo.newParagLayout.leftBorder=leftBorder;
    undoRedoInfo.newParagLayout.rightBorder=rightBorder;
    undoRedoInfo.newParagLayout.topBorder=topBorder;
    undoRedoInfo.newParagLayout.bottomBorder=bottomBorder;

    KoTextParagCommand *cmd = new KoTextParagCommand(
        textdoc, undoRedoInfo.id, undoRedoInfo.eid,
        undoRedoInfo.oldParagLayouts, undoRedoInfo.newParagLayout,
        KoParagLayout::Borders, (Q3StyleSheetItem::Margin)-1, borderOutline);
    textdoc->addCommand( cmd );

    undoRedoInfo.clear();
    emit showCursor();
    emit updateUI( true );
    return new KoTextCommand( this, /*cmd, */i18n("Change Borders") );
}

KCommand * KoTextObject::setJoinBordersCommand( KoTextCursor * cursor, bool join, KoTextDocument::SelectionId selectionId  )
{
  if ( protectContent() )
        return 0L;
  if ( !textdoc->hasSelection( selectionId, true ) &&
       cursor && cursor->parag()->joinBorder() == join )
        return 0L; // No change needed.

    emit hideCursor();
     bool borderOutline = false;
     storeParagUndoRedoInfo( cursor, KoTextDocument::Standard );
    if ( !textdoc->hasSelection( selectionId, true ) )
    {
      cursor->parag()->setJoinBorder( join );
      setLastFormattedParag( cursor->parag() );

      if ( cursor->parag()->next() )
        cursor->parag()->next()->setChanged( true );
      if ( cursor->parag()->prev() )
        cursor->parag()->prev()->setChanged( true );
    }
    else
    {
        KoTextParag *start = textdoc->selectionStart( selectionId );
        KoTextParag *end = textdoc->selectionEnd( selectionId );
        setLastFormattedParag( start );
        for ( ; start && start != end->next() ; start = start->next() )
        {
            start->setJoinBorder( true );
        }
        end->setJoinBorder ( true );
        borderOutline = true;

        if ( start && start->prev() )
            start->prev()->setChanged( true );
        if ( end && end->next() )
            end->next()->setChanged( true );
    }
    formatMore( 2 );

    emit repaintChanged( this );
    undoRedoInfo.newParagLayout.joinBorder=join;

    KoTextParagCommand *cmd = new KoTextParagCommand(
        textdoc, undoRedoInfo.id, undoRedoInfo.eid,
        undoRedoInfo.oldParagLayouts, undoRedoInfo.newParagLayout,
        KoParagLayout::Borders, (Q3StyleSheetItem::Margin)-1, borderOutline);
    textdoc->addCommand( cmd );

    undoRedoInfo.clear();
    emit ensureCursorVisible();
    emit showCursor();
    emit updateUI( true );
    return new KoTextCommand( this, /*cmd, */i18n("Change Join Borders") );
}


KCommand * KoTextObject::setTabListCommand( KoTextCursor * cursor, const KoTabulatorList &tabList, KoTextDocument::SelectionId selectionId  )
{
    if ( protectContent() )
        return 0L;
    if ( !textdoc->hasSelection( selectionId, true ) && cursor &&
         cursor->parag()->tabList() == tabList )
        return 0L; // No change needed.

    emit hideCursor();
    storeParagUndoRedoInfo( cursor, selectionId );

    if ( !textdoc->hasSelection( selectionId, true ) && cursor ) {
        cursor->parag()->setTabList( tabList );
        setLastFormattedParag( cursor->parag() );
    }
    else
    {
        KoTextParag *start = textdoc->selectionStart( selectionId );
        KoTextParag *end = textdoc->selectionEnd( selectionId );
        setLastFormattedParag( start );
        for ( ; start && start != end->next() ; start = start->next() )
            start->setTabList( tabList );
    }

    formatMore( 2 );
    emit repaintChanged( this );
    undoRedoInfo.newParagLayout.setTabList( tabList );
    KoTextParagCommand *cmd = new KoTextParagCommand(
        textdoc, undoRedoInfo.id, undoRedoInfo.eid,
        undoRedoInfo.oldParagLayouts, undoRedoInfo.newParagLayout,
        KoParagLayout::Tabulator);
    textdoc->addCommand( cmd );
    undoRedoInfo.clear();
    emit showCursor();
    emit updateUI( true );
    return new KoTextCommand( this, /*cmd, */i18n("Change Tabulator") );
}

KCommand * KoTextObject::setParagDirectionCommand( KoTextCursor * cursor, QChar::Direction d, KoTextDocument::SelectionId selectionId )
{
    if ( protectContent() )
        return 0L;
    if ( !textdoc->hasSelection( selectionId, true ) && cursor &&
         cursor->parag()->direction() == d )
        return 0L; // No change needed.

    emit hideCursor();
    storeParagUndoRedoInfo( cursor, selectionId );

    if ( !textdoc->hasSelection( selectionId, true ) && cursor ) {
        cursor->parag()->setDirection( d );
        setLastFormattedParag( cursor->parag() );
    }
    else
    {
        KoTextParag *start = textdoc->selectionStart( selectionId );
        KoTextParag *end = textdoc->selectionEnd( selectionId );
        setLastFormattedParag( start );
        for ( ; start && start != end->next() ; start = start->next() )
            start->setDirection( d );
    }

    formatMore( 2 );
    emit repaintChanged( this );
    ////// ### TODO
#if 0
    undoRedoInfo.newParagLayout.direction = d;
    KoTextParagCommand *cmd = new KoTextParagCommand(
        textdoc, undoRedoInfo.id, undoRedoInfo.eid,
        undoRedoInfo.oldParagLayouts, undoRedoInfo.newParagLayout,
        KoParagLayout::Shadow);
    textdoc->addCommand( cmd );
#endif
    undoRedoInfo.clear();
    emit showCursor();
    emit updateUI( true );
#if 0
    return new KoTextCommand( this, /*cmd, */i18n("Change Shadow") );
#else
    return 0L;
#endif
}

void KoTextObject::removeSelectedText( KoTextCursor * cursor, KoTextDocument::SelectionId selectionId, const QString & cmdName, bool createUndoRedo )
{
    if ( protectContent() )
        return ;
    emit hideCursor();
    if( createUndoRedo)
    {
        checkUndoRedoInfo( cursor, UndoRedoInfo::RemoveSelected );
        if ( !undoRedoInfo.valid() ) {
            textdoc->selectionStart( selectionId, undoRedoInfo.id, undoRedoInfo.index );
            undoRedoInfo.text = QString::null;
            newPlaceHolderCommand( cmdName.isNull() ? i18n("Remove Selected Text") : cmdName );
        }
    }
    KoTextCursor c1 = textdoc->selectionStartCursor( selectionId );
    KoTextCursor c2 = textdoc->selectionEndCursor( selectionId );
    readFormats( c1, c2, true, true );
    //kDebug(32500) << "KoTextObject::removeSelectedText text=" << undoRedoInfo.text.toString() << endl;

    textdoc->removeSelectedText( selectionId, cursor );

    setLastFormattedParag( cursor->parag() );
    formatMore( 2 );
    emit repaintChanged( this );
    emit ensureCursorVisible();
    emit updateUI( true );
    emit showCursor();
    if(selectionId==KoTextDocument::Standard || selectionId==KoTextDocument::InputMethodPreedit)
        selectionChangedNotify();
    if ( createUndoRedo)
        undoRedoInfo.clear();
}

KCommand * KoTextObject::removeSelectedTextCommand( KoTextCursor * cursor, KoTextDocument::SelectionId selectionId, bool repaint )
{
    if ( protectContent() )
        return 0L;
    if ( !textdoc->hasSelection( selectionId, true ) )
        return 0L;

    undoRedoInfo.clear();
    textdoc->selectionStart( selectionId, undoRedoInfo.id, undoRedoInfo.index );
    Q_ASSERT( undoRedoInfo.id >= 0 );

    KoTextCursor c1 = textdoc->selectionStartCursor( selectionId );
    KoTextCursor c2 = textdoc->selectionEndCursor( selectionId );
    readFormats( c1, c2, true, true );

    textdoc->removeSelectedText( selectionId, cursor );

    KMacroCommand *macroCmd = new KMacroCommand( i18n("Remove Selected Text") );

    KoTextDocCommand *cmd = deleteTextCommand( textdoc, undoRedoInfo.id, undoRedoInfo.index,
                                                 undoRedoInfo.text.rawData(),
                                                 undoRedoInfo.customItemsMap,
                                                 undoRedoInfo.oldParagLayouts );
    textdoc->addCommand(cmd);
    macroCmd->addCommand(new KoTextCommand( this, /*cmd, */QString::null ));

    if(!undoRedoInfo.customItemsMap.isEmpty())
        undoRedoInfo.customItemsMap.deleteAll( macroCmd );

    undoRedoInfo.type = UndoRedoInfo::Invalid; // we don't want clear() to create a command
    undoRedoInfo.clear();
    if ( repaint )
        selectionChangedNotify();
    return macroCmd;
}

KCommand* KoTextObject::replaceSelectionCommand( KoTextCursor * cursor, const QString & replacement,
                                                 const QString & cmdName,
                                                 KoTextDocument::SelectionId selectionId,
                                                 int insertFlags,
                                                 CustomItemsMap customItemsMap )
{
    if ( protectContent() )
        return 0L;
    Q_ASSERT( ( insertFlags & DoNotRemoveSelected ) == 0 ); // nonsensical
    const bool repaint = ( insertFlags & DoNotRepaint ) == 0; // DoNotRepaint is set during search/replace
    if ( repaint )
        emit hideCursor();
    // This could be improved to use a macro command only when there's a selection to remove.
    KMacroCommand * macroCmd = new KMacroCommand( cmdName );

    // Remember formatting
    KoTextCursor c1 = textdoc->selectionStartCursor( selectionId );
    KoTextFormat * format = c1.parag()->at( c1.index() )->format();
    format->addRef();

    // Remove selected text, if any
    KCommand* removeSelCmd = removeSelectedTextCommand( cursor, selectionId, repaint );
    if ( removeSelCmd )
        macroCmd->addCommand( removeSelCmd );

    // Insert replacement
    insert( cursor, format,
            replacement, QString::null /* no place holder command */,
            selectionId, insertFlags | DoNotRemoveSelected, customItemsMap );

    KoTextDocCommand * cmd = new KoTextInsertCommand( textdoc, undoRedoInfo.id, undoRedoInfo.index,
                                                      undoRedoInfo.text.rawData(),
                                                      CustomItemsMap(), undoRedoInfo.oldParagLayouts );
    textdoc->addCommand( cmd );
    macroCmd->addCommand( new KoTextCommand( this, /*cmd, */QString::null ) );

    undoRedoInfo.type = UndoRedoInfo::Invalid; // we don't want clear() to create a command
    undoRedoInfo.clear();

    format->removeRef();

    setLastFormattedParag( c1.parag() );
    if ( repaint )
    {
        formatMore( 2 );
        emit repaintChanged( this );
        emit ensureCursorVisible();
        emit updateUI( true );
        emit showCursor();
        if(selectionId==KoTextDocument::Standard)
            selectionChangedNotify();
    }
    return macroCmd;
}

KCommand * KoTextObject::insertParagraphCommand( KoTextCursor *cursor )
{
    if ( protectContent() )
        return 0L;
    return replaceSelectionCommand( cursor, "\n", QString::null );
}

void KoTextObject::highlightPortion( KoTextParag * parag, int index, int length, bool repaint )
{
    if ( !m_highlightSelectionAdded )
    {
        textdoc->addSelection( KoTextDocument::HighlightSelection );
        textdoc->setSelectionColor( KoTextDocument::HighlightSelection,
                                    QApplication::palette().color( QPalette::Active, QColorGroup::Dark ) );
        textdoc->setInvertSelectionText( KoTextDocument::HighlightSelection, true );
        m_highlightSelectionAdded = true;
    }

    removeHighlight(repaint); // remove previous highlighted selection
    KoTextCursor cursor( textdoc );
    cursor.setParag( parag );
    cursor.setIndex( index );
    textdoc->setSelectionStart( KoTextDocument::HighlightSelection, &cursor );
    cursor.setIndex( index + length );
    textdoc->setSelectionEnd( KoTextDocument::HighlightSelection, &cursor );
    if ( repaint ) {
        parag->setChanged( true );
        emit repaintChanged( this );
    }
}

void KoTextObject::removeHighlight(bool repaint)
{
    if ( textdoc->hasSelection( KoTextDocument::HighlightSelection, true ) )
    {
        KoTextParag * oldParag = textdoc->selectionStart( KoTextDocument::HighlightSelection );
        oldParag->setChanged( true );
        textdoc->removeSelection( KoTextDocument::HighlightSelection );
    }
    if ( repaint )
        emit repaintChanged( this );
}

void KoTextObject::selectAll( bool select )
{
    if ( !select )
        textdoc->removeSelection( KoTextDocument::Standard );
    else
        textdoc->selectAll( KoTextDocument::Standard );
    selectionChangedNotify();
}

void KoTextObject::selectionChangedNotify( bool enableActions /* = true */)
{
    emit repaintChanged( this );
    if ( enableActions )
        emit selectionChanged( hasSelection() );
}

void KoTextObject::setViewArea( QWidget* w, int maxY )
{
    m_mapViewAreas.replace( w, maxY );
}

void KoTextObject::setLastFormattedParag( KoTextParag *parag )
{
    //kDebug() << k_funcinfo << parag << " (" << ( parag ? QString::number(parag->paragId()) : QString("(null)") ) << ")" << endl;
    if ( !m_lastFormatted || !parag || m_lastFormatted->paragId() >= parag->paragId() ) {
        m_lastFormatted = parag;
    }
}

void KoTextObject::ensureFormatted( KoTextParag * parag, bool emitAfterFormatting /* = true */ )
{
    if ( !textdoc->lastParag() )
        return; // safety test
    //kDebug(32500) << name() << " ensureFormatted " << parag->paragId() << endl;
    if ( !parag->isValid() && m_lastFormatted == 0 )
        m_lastFormatted = parag; // bootstrap

    while ( !parag->isValid() )
    {
        if ( !m_lastFormatted ) {
            kWarning() << "ensureFormatted for parag " << parag << " " << parag->paragId() << " still not formatted, but m_lastFormatted==0" << endl;
            return;
        }
        // The paragid diff is "a good guess". The >=1 is a safety measure ;)
        bool ret = formatMore( qMax( 1, parag->paragId() - m_lastFormatted->paragId() ), emitAfterFormatting );
        if ( !ret ) { // aborted
            //kDebug(32500) << "ensureFormatted aborted!" << endl;
            break;
        }
    }
    //kDebug(32500) << name() << " ensureFormatted " << parag->paragId() << " done" << endl;
}

bool KoTextObject::formatMore( int count /* = 10 */, bool emitAfterFormatting /* = true */ )
{
    if ( ( !m_lastFormatted && d->afterFormattingEmitted )
         || !m_visible || m_availableHeight == -1 )
        return false;

    if ( !textdoc->lastParag() )
        return false; // safety test

    if ( d->abortFormatting ) {
        d->abortFormatting = false;
        return false;
    }

    if ( count == 0 )
    {
        formatTimer->start( interval, TRUE );
        return true;
    }

    int bottom = 0;
    if ( m_lastFormatted )
    {
        d->afterFormattingEmitted = false;

        int viewsBottom = 0;
        QMap<QWidget *, int>::const_iterator mapIt = m_mapViewAreas.begin();
        for ( ; mapIt != m_mapViewAreas.end() ; ++mapIt )
            viewsBottom = qMax( viewsBottom, mapIt.value() );

#ifdef DEBUG_FORMAT_MORE
        kDebug(32500) << "formatMore " << name()
                       << " lastFormatted id=" << m_lastFormatted->paragId()
                       << " lastFormatted's top=" << m_lastFormatted->rect().top()
                       << " lastFormatted's height=" << m_lastFormatted->rect().height()
                       << " count=" << count << " viewsBottom=" << viewsBottom
                       << " availableHeight=" << m_availableHeight << endl;
#endif
        if ( m_lastFormatted->prev() == 0 )
        {
            emit formattingFirstParag();
#ifdef TIMING_FORMAT
            kDebug(32500) << "formatMore " << name() << ". First parag -> starting timer" << endl;
            m_time.start();
#endif
        }

        // Stop if we have formatted everything or if we need more space
        // Otherwise, stop formatting after "to" paragraphs,
        // but make sure we format everything the views need
        int i;
        for ( i = 0;
              m_lastFormatted && bottom + m_lastFormatted->rect().height() <= m_availableHeight &&
                  ( i < count || bottom <= viewsBottom ) ; ++i )
        {
            KoTextParag* parag = m_lastFormatted;
#ifdef DEBUG_FORMAT_MORE
            kDebug(32500) << "formatMore formatting " << parag << " id=" << parag->paragId() << endl;
#endif
            assert( parag->string() ); // i.e. not deleted
            parag->format();
            bottom = parag->rect().top() + parag->rect().height();
#if 0 //def DEBUG_FORMAT_MORE
            kDebug(32500) << "formatMore(inside) top=" << parag->rect().top()
                      << " height=" << parag->rect().height()
                      << " bottom=" << bottom << " m_lastFormatted(next parag) = " << m_lastFormatted->next() << endl;
#endif

            // Check for Head 1 (i.e. section) titles, and emit them - for the Section variable
            if ( parag->counter() && parag->counter()->numbering() == KoParagCounter::NUM_CHAPTER
                 && parag->counter()->depth() == 0 )
                emit chapterParagraphFormatted( parag );

            if ( d->abortFormatting ) {
#ifdef DEBUG_FORMAT_MORE
                kDebug(32500) << "formatMore formatting aborted. " << endl;
#endif
                d->abortFormatting = false;
                return false;
            }

            if ( parag != m_lastFormatted )
                kWarning() << "Some code changed m_lastFormatted during formatting! Was " << parag->paragId() << ", is now " << m_lastFormatted->paragId() << endl;
#if 0 // This happens now that formatting can 'abort' (e.g. due to page not yet created)
            else if (!parag->isValid())
                kWarning() << "PARAGRAPH " << parag->paragId() << " STILL INVALID AFTER FORMATTING" << endl;
#endif
            m_lastFormatted = parag->next();
        }
    }
    else // formatting was done previously, but not emit afterFormatting
    {
        QRect rect = textdoc->lastParag()->rect();
        bottom = rect.top() + rect.height();
    }
#ifdef DEBUG_FORMAT_MORE
    QString id;
    if ( m_lastFormatted ) id = QString(" (%1)").arg(m_lastFormatted->paragId());
    kDebug(32500) << "formatMore finished formatting. "
                   << " bottom=" << bottom
                   << " m_lastFormatted=" << m_lastFormatted << id
                   << endl;
#endif

    if ( emitAfterFormatting )
    {
        d->afterFormattingEmitted = true;
        bool needMoreSpace = false;
        // Check if we need more space
        if ( ( bottom > m_availableHeight ) ||   // this parag is already off page
             ( m_lastFormatted && bottom + m_lastFormatted->rect().height() > m_availableHeight ) ) // or next parag will be off page
            needMoreSpace = true;
        // default value of 'abort' depends on need more space
        bool abort = needMoreSpace;
        emit afterFormatting( bottom, m_lastFormatted, &abort );
        if ( abort )
            return false;
        else if ( needMoreSpace && m_lastFormatted ) // we got more space, keep formatting then
            return formatMore( 2 );
    }

    // Now let's see when we'll need to get back here.
    if ( m_lastFormatted )
    {
        formatTimer->start( interval, TRUE );
#ifdef DEBUG_FORMAT_MORE
        kDebug(32500) << name() << " formatMore: will have to format more. formatTimer->start with interval=" << interval << endl;
#endif
    }
    else
    {
        interval = qMax( 0, interval );
#ifdef DEBUG_FORMAT_MORE
        kDebug(32500) << name() << " formatMore: all formatted interval=" << interval << endl;
#endif
#ifdef TIMING_FORMAT
        //if ( frameSetInfo() == FI_BODY )
        kDebug(32500) << "formatMore: " << name() << " all formatted. Took "
                       << (double)(m_time.elapsed()) / 1000 << " seconds." << endl;
#endif
    }
    return true;
}

void KoTextObject::abortFormatting()
{
    d->abortFormatting = true;
}

void KoTextObject::doChangeInterval()
{
    //kDebug(32500) << "KoTextObject::doChangeInterval back to interval=0" << endl;
    interval = 0;
}

void KoTextObject::typingStarted()
{
    //kDebug(32500) << "KoTextObject::typingStarted" << endl;
    startTimer->stop();
    interval = 10;
}

void KoTextObject::typingDone()
{
    startTimer->start( 100, TRUE );
}


// helper for changeCaseOfText
KCommand *KoTextObject::changeCaseOfTextParag( int cursorPosStart, int cursorPosEnd,
                                               KoChangeCaseDia::TypeOfCase _type,
                                               KoTextCursor *cursor, KoTextParag *parag )
{
    if ( protectContent() )
        return 0L;

    KMacroCommand * macroCmd = new KMacroCommand( i18n("Change Case") );
    KoTextFormat *curFormat = parag->paragraphFormat();
    const QString text = parag->string()->toString().mid( cursorPosStart, cursorPosEnd - cursorPosStart );
    int posStart = cursorPosStart;
    int posEnd = cursorPosStart;
    KoTextCursor c1( textdoc );
    KoTextCursor c2( textdoc );
    //kDebug() << k_funcinfo << "from " << cursorPosStart << " to " << cursorPosEnd << " (excluded). Parag=" << parag << " length=" << parag->length() << endl;
    for ( int i = cursorPosStart; i < cursorPosEnd; ++i )
    {
        KoTextStringChar & ch = *(parag->at(i));
        KoTextFormat * newFormat = ch.format();
        if( ch.isCustom() )
        {
            posEnd = i;
            c1.setParag( parag );
            c1.setIndex( posStart );
            c2.setParag( parag );
            c2.setIndex( posEnd );
            //kDebug() << k_funcinfo << "found custom at " << i << " : doing from " << posStart << " to " << posEnd << endl;

            const QString repl = text.mid( posStart, posEnd - posStart );
            textdoc->setSelectionStart( KoTextDocument::Temp, &c1 );
            textdoc->setSelectionEnd( KoTextDocument::Temp, &c2 );
            macroCmd->addCommand(replaceSelectionCommand(
                                     cursor, textChangedCase(repl,_type),
                                     QString::null, KoTextDocument::Temp));
            do
            {
                ++i;
            }
            while( parag->at(i)->isCustom() && i != cursorPosEnd);
            posStart=i;
            posEnd=i;
        }
        else
        {
            if ( newFormat != curFormat )
            {
                posEnd=i;
                c1.setParag( parag );
                c1.setIndex( posStart );
                c2.setParag( parag );
                c2.setIndex( posEnd );
                //kDebug() << k_funcinfo << "found new format at " << i << " : doing from " << posStart << " to " << posEnd << endl;

                const QString repl = text.mid( posStart, posEnd - posStart );
                textdoc->setSelectionStart( KoTextDocument::Temp, &c1 );
                textdoc->setSelectionEnd( KoTextDocument::Temp, &c2 );
                macroCmd->addCommand(replaceSelectionCommand(
                                         cursor, textChangedCase(repl,_type),
                                         QString::null, KoTextDocument::Temp));
                posStart=i;
                posEnd=i;
                curFormat = newFormat;
            }
        }
    }
    if ( posStart != cursorPosEnd )
    {
        //kDebug() << k_funcinfo << "finishing: doing from " << posStart << " to " << cursorPosEnd << endl;
        c1.setParag( parag );
        c1.setIndex( posStart );
        c2.setParag( parag );
        c2.setIndex( cursorPosEnd );

        textdoc->setSelectionStart( KoTextDocument::Temp, &c1 );
        textdoc->setSelectionEnd( KoTextDocument::Temp, &c2 );
        const QString repl = text.mid( posStart, cursorPosEnd - posStart );
        macroCmd->addCommand(replaceSelectionCommand(
                                 cursor, textChangedCase(repl,_type),
                                 QString::null, KoTextDocument::Temp));
    }
    return macroCmd;

}

KCommand *KoTextObject::changeCaseOfText(KoTextCursor *cursor,KoChangeCaseDia::TypeOfCase _type)
{
    if ( protectContent() )
        return 0L;
    KMacroCommand * macroCmd = new KMacroCommand( i18n("Change Case") );

    KoTextCursor start = textdoc->selectionStartCursor( KoTextDocument::Standard );
    KoTextCursor end = textdoc->selectionEndCursor( KoTextDocument::Standard );
    emit hideCursor();

    if ( start.parag() == end.parag() )
    {
        int endIndex = qMin( start.parag()->length() - 1, end.index() );
        macroCmd->addCommand( changeCaseOfTextParag( start.index(), endIndex, _type,
                                                     cursor, start.parag() ) );
    }
    else
    {
        macroCmd->addCommand( changeCaseOfTextParag( start.index(), start.parag()->length() - 1, _type,
                                                     cursor, start.parag() ) );
        KoTextParag *p = start.parag()->next();
        while ( p && p != end.parag() )
        {
            macroCmd->addCommand( changeCaseOfTextParag(0, p->length() - 1, _type, cursor, p ) );
            p = p->next();
        }
        if ( p )
        {
            int endIndex = qMin( p->length() - 1, end.index() );
            macroCmd->addCommand( changeCaseOfTextParag(0, endIndex, _type, cursor, end.parag() ));
        }
    }
    formatMore( 2 );
    emit repaintChanged( this );
    emit ensureCursorVisible();
    emit updateUI( true );
    emit showCursor();
    return macroCmd;
}

QString KoTextObject::textChangedCase(const QString& _text,KoChangeCaseDia::TypeOfCase _type)
{
    QString text(_text);
    switch(_type)
    {
        case KoChangeCaseDia::UpperCase:
            text=text.upper();
            break;
        case KoChangeCaseDia::LowerCase:
            text=text.lower();
            break;
        case KoChangeCaseDia::TitleCase:
            for(int i=0;i<text.length();i++)
            {
                if(text.at(i)!=' ')
                {
                    QChar prev = text.at(qMax(i-1,0));
                    if(i==0 || prev  == ' ' || prev == '\n' || prev == '\t')
                        text=text.replace(i, 1, text.at(i).upper() );
                    else
                        text=text.replace(i, 1, text.at(i).lower() );
                }
            }
            break;
        case KoChangeCaseDia::ToggleCase:
            for(uint i=0;i<text.length();i++)
            {
                QString repl=QString(text.at(i));
                if(text.at(i)!=text.at(i).upper())
                    repl=repl.upper();
                else if(text.at(i).lower()!=text.at(i))
                    repl=repl.lower();
                text=text.replace(i, 1, repl );
            }
            break;
        case KoChangeCaseDia::SentenceCase:
            for(int i=0;i<text.length();i++)
            {
                if(text.at(i)!=' ')
                {
                    QChar prev = text.at(qMax(i-1,0));
                    if(i==0 || prev == '\n' ||prev.isPunct())
                        text=text.replace(i, 1, text.at(i).upper() );
                }
            }
            break;
        default:
            kDebug(32500)<<"Error in changeCaseOfText !\n";
            break;

    }
    return text;
}

// Warning, this doesn't ref the format!
KoTextFormat * KoTextObject::currentFormat() const
{
    // We use the formatting of the very first character
    // Should we use a style instead, maybe ?
    KoTextStringChar *ch = textdoc->firstParag()->at( 0 );
    return ch->format();
}

const KoParagLayout * KoTextObject::currentParagLayoutFormat() const
{
    KoTextParag * parag = textdoc->firstParag();
    return &(parag->paragLayout());
}

bool KoTextObject::rtl() const
{
    return textdoc->firstParag()->string()->isRightToLeft();
}

void KoTextObject::loadOasisContent( const QDomElement &bodyElem, KoOasisContext& context, KoStyleCollection * styleColl )
{
    textDocument()->clear(false); // Get rid of dummy paragraph (and more if any)
    setLastFormattedParag( 0L ); // no more parags, avoid UMR in next setLastFormattedParag call

    KoTextParag *lastParagraph = textDocument()->loadOasisText( bodyElem, context, 0, styleColl, 0 );

    if ( !lastParagraph )                // We created no paragraph
    {
        // Create an empty one, then. See KoTextDocument ctor.
        textDocument()->clear( true );
        textDocument()->firstParag()->setStyle( styleColl->findStyle( "Standard" ) );
    }
    else
        textDocument()->setLastParag( lastParagraph );

    setLastFormattedParag( textDocument()->firstParag() );
}

KoTextCursor KoTextObject::pasteOasisText( const QDomElement &bodyElem, KoOasisContext& context,
                                           KoTextCursor& cursor, KoStyleCollection * styleColl )
{
    KoTextCursor resultCursor( cursor );
    KoTextParag* lastParagraph = cursor.parag();
    bool removeNewline = false;
    uint pos = cursor.index();
    if ( pos == 0 && lastParagraph->length() <= 1 ) {
        // Pasting on an empty paragraph -> respect <text:h> in selected text etc.
        lastParagraph = lastParagraph->prev();
        lastParagraph = textDocument()->loadOasisText( bodyElem, context, lastParagraph, styleColl, cursor.parag() );
        if ( lastParagraph ) {
            resultCursor.setParag( lastParagraph );
            resultCursor.setIndex( lastParagraph->length() - 1 );
        }
        removeNewline = true;
    } else {
        // Pasting inside a non-empty paragraph -> insert/append text to it.
        // This loop looks for the *FIRST* paragraph only.
        for ( QDomNode text (bodyElem.firstChild()); !text.isNull(); text = text.nextSibling() )
        {
            QDomElement tag = text.toElement();
            if ( tag.isNull() ) continue;
            // The first tag could be a text:p, text:h, text:numbered-paragraph, but also
            // a frame (anchored to page), a TOC, etc. For those, don't try to concat-with-existing-paragraph.
            // For text:numbered-paragraph, find the text:p or text:h inside it.
            QDomElement tagToLoad = tag;
            if ( tag.localName() == "numbered-paragraph" ) {
                QDomElement npchild;
                forEachElement( npchild, tag )
                {
                    if ( npchild.localName() == "p" || npchild.localName() == "h" ) {
                        tagToLoad = npchild;
                        break;
                    }
                }
            }

            if ( tagToLoad.localName() == "p" || tagToLoad.localName() == "h" ) {
                context.styleStack().save();
                context.fillStyleStack( tagToLoad, KoXmlNS::text, "style-name", "paragraph" );
                lastParagraph->loadOasisSpan( tagToLoad, context, pos );
                context.styleStack().restore();

                lastParagraph->setChanged( true );
                lastParagraph->invalidate( 0 );

                // Now split this parag, to make room for the next paragraphs
                resultCursor.setParag( lastParagraph );
                resultCursor.setIndex( pos );
                resultCursor.splitAndInsertEmptyParag( FALSE, TRUE );
                removeNewline = true;

                // Done with first parag, remove it and exit loop
                const_cast<QDomElement &>( bodyElem ).removeChild( tag ); // somewhat hackish
            }
            break;
        }
        resultCursor.setParag( lastParagraph );
        resultCursor.setIndex( pos );
        // Load the rest the usual way.
        lastParagraph = textDocument()->loadOasisText( bodyElem, context, lastParagraph, styleColl, lastParagraph->next() );
        if ( lastParagraph != resultCursor.parag() ) // we loaded more paragraphs
        {
            removeNewline = true;
            resultCursor.setParag( lastParagraph );
            resultCursor.setIndex( lastParagraph->length() - 1 );
        }
    }
    KoTextParag* p = resultCursor.parag();
    if ( p ) p = p->next();
    // Remove the additional newline that loadOasisText inserted
    if ( removeNewline && resultCursor.remove() ) {
        if ( m_lastFormatted == p ) { // has been deleted
            m_lastFormatted = resultCursor.parag();
        }
    }
    return resultCursor;
}

void KoTextObject::saveOasisContent( KoXmlWriter& writer, KoSavingContext& context ) const
{
    textDocument()->saveOasisContent( writer, context );
}

KCommand *KoTextObject::setParagLayoutFormatCommand( KoParagLayout *newLayout,int flags,int marginIndex)
{
    if ( protectContent() )
        return 0L;
    textdoc->selectAll( KoTextDocument::Temp );
    KoTextCursor *cursor = new KoTextCursor( textdoc );
    KCommand* cmd = setParagLayoutCommand( cursor, *newLayout, KoTextDocument::Temp,
                                           flags, marginIndex, true /*createUndoRedo*/ );
    textdoc->removeSelection( KoTextDocument::Temp );
    delete cursor;
    return cmd;
}

void KoTextObject::setFormat( KoTextFormat * newFormat, int flags, bool zoomFont )
{
    if ( protectContent() )
        return ;
    // This version of setFormat works on the whole textobject - we use the Temp selection for that
    textdoc->selectAll( KoTextDocument::Temp );
    KCommand *cmd = setFormatCommand( 0L, 0L, newFormat,
                                      flags, zoomFont, KoTextDocument::Temp );
    textdoc->removeSelection( KoTextDocument::Temp );
    if (cmd)
        emit newCommand( cmd );

    KoTextFormat format = *currentFormat();
    //format.setPointSize( docFontSize( currentFormat() ) ); // "unzoom" the font size
    emit showFormatObject(format);
}

KCommand *KoTextObject::setChangeCaseOfTextCommand(KoChangeCaseDia::TypeOfCase _type)
{
    if ( protectContent() )
        return 0L;
    textdoc->selectAll( KoTextDocument::Standard );
    KoTextCursor *cursor = new KoTextCursor( textdoc );
    KCommand* cmd = changeCaseOfText(cursor, _type);
    textdoc->removeSelection( KoTextDocument::Standard );
    delete cursor;
    return cmd;
}

void KoTextObject::setNeedSpellCheck(bool b)
{
    m_needsSpellCheck = b;
    for (KoTextParag * parag = textdoc->firstParag(); parag ; parag = parag->next())
        parag->string()->setNeedsSpellCheck( b );
}

bool KoTextObject::statistics( Q3ProgressDialog *progress, ulong & charsWithSpace, ulong & charsWithoutSpace, ulong & words, ulong & sentences, ulong & syllables, ulong & lines, bool selected )
{
    // parts of words for better counting of syllables:
    // (only use reg exp if necessary -> speed up)

    QStringList subs_syl;
    subs_syl << "cial" << "tia" << "cius" << "cious" << "giu" << "ion" << "iou";
    QStringList subs_syl_regexp;
    subs_syl_regexp << "sia$" << "ely$";

    QStringList add_syl;
    add_syl << "ia" << "riet" << "dien" << "iu" << "io" << "ii";
    QStringList add_syl_regexp;
    add_syl_regexp << "[aeiouym]bl$" << "[aeiou]{3}" << "^mc" << "ism$"
        << "[^l]lien" << "^coa[dglx]." << "[^gq]ua[^auieo]" << "dnt$";

    QString s;
    KoTextParag * parag = textdoc->firstParag();
    for ( ; parag ; parag = parag->next() )
    {
        if (  progress )
        {
            progress->setProgress(progress->progress()+1);
            // MA: resizing if KWStatisticsDialog does not work correct with this enabled, don't know why
            kapp->processEvents();
            if ( progress->wasCanceled() )
                return false;
        }
        // start of a table
/*        if ( parag->at(0)->isCustom())
        {
            KoLinkVariable *var=dynamic_cast<KoLinkVariable *>(parag->at(0)->customItem());
            if(!var)
                continue;
                }*/
        bool hasTrailingSpace = true;
        if ( !selected ) {
            s = parag->string()->toString();
            lines += parag->lines();
        } else {
            if ( parag->hasSelection( KoTextDocument::Standard ) ) {
                hasTrailingSpace = false;
                s = parag->string()->toString();
                if ( !( parag->fullSelected( KoTextDocument::Standard ) ) ) {
                    s = s.mid( parag->selectionStart( KoTextDocument::Standard ), parag->selectionEnd( KoTextDocument::Standard ) - parag->selectionStart( KoTextDocument::Standard ) );
                    lines+=numberOfparagraphLineSelected(parag);
                }
                else
                    lines += parag->lines();
            } else {
                continue;
            }
        }

        // Character count
        for ( uint i = 0 ; i < s.length() - ( hasTrailingSpace ? 1 : 0 ) ; ++i )
        {
            QChar ch = s[i];
            ++charsWithSpace;
            if ( !ch.isSpace() )
                ++charsWithoutSpace;
        }

        // Syllable and Word count
        // Algorithm mostly taken from Greg Fast's Lingua::EN::Syllable module for Perl.
        // This guesses correct for 70-90% of English words, but the overall value
        // is quite good, as some words get a number that's too high and others get
        // one that's too low.
        // IMPORTANT: please test any changes against demos/statistics.kwd
        QRegExp re("\\s+");
        QStringList wordlist = QStringList::split(re, s);
        words += wordlist.count();
        re.setCaseSensitive(false);
        for ( QStringList::Iterator it = wordlist.begin(); it != wordlist.end(); ++it ) {
            QString word = *it;
            re.setPattern("[!?.,:_\"-]");    // clean word from punctuation
            word.remove(re);
            if ( word.length() <= 3 ) {  // extension to the original algorithm
                syllables++;
                continue;
            }
            re.setPattern("e$");
            word.remove(re);
            re.setPattern("[^aeiouy]+");
            QStringList syls = QStringList::split(re, word);
            int word_syllables = 0;
            for ( QStringList::Iterator it = subs_syl.begin(); it != subs_syl.end(); ++it ) {
                if( word.find(*it, 0, false) != -1 )
                    word_syllables--;
            }
            for ( QStringList::Iterator it = subs_syl_regexp.begin(); it != subs_syl_regexp.end(); ++it ) {
                re.setPattern(*it);
                if( word.find(re) != -1 )
                    word_syllables--;
            }
            for ( QStringList::Iterator it = add_syl.begin(); it != add_syl.end(); ++it ) {
                if( word.find(*it, 0, false) != -1 )
                    word_syllables++;
            }
            for ( QStringList::Iterator it = add_syl_regexp.begin(); it != add_syl_regexp.end(); ++it ) {
                re.setPattern(*it);
                if( word.find(re) != -1 )
                    word_syllables++;
            }
            word_syllables += syls.count();
            if ( word_syllables == 0 )
                word_syllables = 1;
            syllables += word_syllables;
        }
        re.setCaseSensitive(true);

        // Sentence count
        // Clean up for better result, destroys the original text but we only want to count
        s = s.trimmed();
        QChar lastchar = s.at(s.length());
        if( ! s.isEmpty() && ! KoAutoFormat::isMark( lastchar ) ) {  // e.g. for headlines
            s = s + ".";
        }
        re.setPattern("[.?!]+");         // count "..." as only one "."
        s.replace(re, ".");
        re.setPattern("\\d\\.\\d");      // don't count floating point numbers as sentences
        s.replace(re, "0,0");
        re.setPattern("[A-Z]\\.+");      // don't count "U.S.A." as three sentences
        s.replace(re, "*");
        for ( uint i = 0 ; i < s.length() ; ++i )
        {
            QChar ch = s[i];
            if ( KoAutoFormat::isMark( ch ) )
                ++sentences;
        }
    }
    return true;
}

int KoTextObject::numberOfparagraphLineSelected( KoTextParag *parag)
{
    int indexOfLineStart;
    int lineStart;
    int lineEnd;
    KoTextCursor c1 = textdoc->selectionStartCursor( KoTextDocument::Standard );
    KoTextCursor c2 = textdoc->selectionEndCursor( KoTextDocument::Standard );
    parag->lineStartOfChar( c1.index(), &indexOfLineStart, &lineStart );

    parag->lineStartOfChar( c2.index(), &indexOfLineStart, &lineEnd );
    return (lineEnd - lineStart+1);
}

KoVariable* KoTextObject::variableAtPoint( const QPoint& iPoint ) const
{
    KoTextCursor cursor( textDocument() );
    int variablePosition = -1;
    cursor.place( iPoint, textDocument()->firstParag(), false, &variablePosition );
    if ( variablePosition == -1 )
        return 0;
    return variableAtPosition( cursor.parag(), variablePosition );
}

KoVariable* KoTextObject::variableAtPosition( KoTextParag* parag, int index ) const
{
    KoTextStringChar * ch = parag->at( index );
    if ( ch->isCustom() )
        return dynamic_cast<KoVariable *>( ch->customItem() );
    return 0;
}

const char * KoTextObject::acceptSelectionMimeType()
{
    return "application/vnd.oasis.opendocument.";
}

Q3CString KoTextObject::providesOasis( QMimeSource* mime )
{
    const char* fmt;
    const char* acceptMimeType = acceptSelectionMimeType();
    for ( int i = 0; (fmt = mime->format(i)); ++i )
    {
        if ( QString( fmt ).startsWith( acceptMimeType ) )
        {
            return fmt;
        }
    }
    return "";
}

#ifndef NDEBUG
void KoTextObject::printRTDebug(int info)
{
    KoTextParag* lastParag = 0;
    for (KoTextParag * parag = textdoc->firstParag(); parag ; parag = parag->next())
    {
        assert( parag->prev() == lastParag );
        parag->printRTDebug( info );
        lastParag = parag;
    }
    if ( info == 1 )
        textdoc->formatCollection()->debug();
}
#endif

////// Implementation of the KoTextFormatInterface "interface"

KCommand *KoTextFormatInterface::setBoldCommand(bool on)
{
    KoTextFormat format( *currentFormat() );
    format.setBold( on );
    return setFormatCommand( &format, KoTextFormat::Bold );
}

KCommand *KoTextFormatInterface::setItalicCommand( bool on)
{
    KoTextFormat format( *currentFormat() );
    format.setItalic( on );
    return setFormatCommand( &format, KoTextFormat::Italic );
}

KCommand *KoTextFormatInterface::setUnderlineCommand( bool on )
{
    KoTextFormat format( *currentFormat() );
    format.setUnderlineType( on ? KoTextFormat::U_SIMPLE : KoTextFormat::U_NONE);
    return setFormatCommand( &format, KoTextFormat::ExtendUnderLine );
}

KCommand *KoTextFormatInterface::setUnderlineColorCommand( const QColor &color )
{
    KoTextFormat format( *currentFormat() );
    format.setTextUnderlineColor( color);
    return setFormatCommand( &format, KoTextFormat::ExtendUnderLine );
}

KCommand *KoTextFormatInterface::setDoubleUnderlineCommand( bool on )
{
    KoTextFormat format( *currentFormat() );
    format.setUnderlineType( on ? KoTextFormat::U_DOUBLE : KoTextFormat::U_NONE);
    return setFormatCommand( &format, KoTextFormat::ExtendUnderLine );
}

KCommand *KoTextFormatInterface::setStrikeOutCommand( bool on )
{
    KoTextFormat format( *currentFormat() );
    format.setStrikeOutType( on ? KoTextFormat::S_SIMPLE : KoTextFormat::S_NONE);
    return setFormatCommand( &format, KoTextFormat::StrikeOut );
}

KCommand *KoTextFormatInterface::setTextBackgroundColorCommand(const QColor & col)
{
    KoTextFormat format( *currentFormat() );
    format.setTextBackgroundColor( col );
    return setFormatCommand( &format, KoTextFormat::TextBackgroundColor );
}

KCommand *KoTextFormatInterface::setPointSizeCommand( int s )
{
    KoTextFormat format( *currentFormat() );
    format.setPointSize( s );
    return setFormatCommand( &format, KoTextFormat::Size, true /* zoom the font size */ );
}

KCommand *KoTextFormatInterface::setFamilyCommand(const QString &font)
{
    KoTextFormat format( *currentFormat() );
    format.setFamily( font );
    return setFormatCommand( &format, KoTextFormat::Family );
}

QColor KoTextFormatInterface::textBackgroundColor() const
{
    return currentFormat()->textBackgroundColor();
}

QColor KoTextFormatInterface::textUnderlineColor()const
{
    return currentFormat()->textUnderlineColor();
}

QColor KoTextFormatInterface::textColor() const
{
    return currentFormat()->color();
}

bool KoTextFormatInterface::textUnderline()const
{
    return currentFormat()->underline();
}

bool KoTextFormatInterface::textDoubleUnderline()const
{
    return currentFormat()->doubleUnderline();
}

bool KoTextFormatInterface::textBold()const
{
    return currentFormat()->font().bold();
}

bool KoTextFormatInterface::textStrikeOut()const
{
    return currentFormat()->font().strikeOut();
}

bool KoTextFormatInterface::textItalic() const
{
    return currentFormat()->font().italic();
}

bool KoTextFormatInterface::textSubScript() const
{
    return (currentFormat()->vAlign()==KoTextFormat::AlignSubScript);
}

bool KoTextFormatInterface::textSuperScript() const
{
    return (currentFormat()->vAlign()==KoTextFormat::AlignSuperScript);
}

double KoTextFormatInterface::shadowDistanceX() const
{
    return currentFormat()->shadowDistanceX();
}

double KoTextFormatInterface::shadowDistanceY() const
{
    return currentFormat()->shadowDistanceY();
}

QColor KoTextFormatInterface::shadowColor() const
{
    return currentFormat()->shadowColor();
}

KoTextFormat::AttributeStyle KoTextFormatInterface::fontAttribute() const
{
    return currentFormat()->attributeFont();
}

double KoTextFormatInterface::relativeTextSize() const
{
    return currentFormat()->relativeTextSize();
}

int KoTextFormatInterface::offsetFromBaseLine()const
{
    return currentFormat()->offsetFromBaseLine();
}

bool KoTextFormatInterface::wordByWord()const
{
    return currentFormat()->wordByWord();
}

bool KoTextFormatInterface::hyphenation()const
{
    return currentFormat()->hyphenation();
}

KoTextFormat::UnderlineType KoTextFormatInterface::underlineType()const
{
    return currentFormat()->underlineType();
}

KoTextFormat::StrikeOutType KoTextFormatInterface::strikeOutType()const
{
    return currentFormat()->strikeOutType();
}

KoTextFormat::UnderlineStyle KoTextFormatInterface::underlineStyle()const
{
    return currentFormat()->underlineStyle();
}

KoTextFormat::StrikeOutStyle KoTextFormatInterface::strikeOutStyle()const
{
    return currentFormat()->strikeOutStyle();
}

QFont KoTextFormatInterface::textFont() const
{
    QFont fn( currentFormat()->font() );
    // "unzoom" the font size
    //fn.setPointSize( static_cast<int>( KoTextZoomHandler::layoutUnitPtToPt( fn.pointSize() ) ) );
    return fn;
}

QString KoTextFormatInterface::textFontFamily()const
{
    return currentFormat()->font().family();
}

QString KoTextFormatInterface::language() const
{
    return currentFormat()->language();
}

KCommand *KoTextFormatInterface::setTextColorCommand(const QColor &color)
{
    KoTextFormat format( *currentFormat() );
    format.setColor( color );
    return setFormatCommand( &format, KoTextFormat::Color );
}

KCommand *KoTextFormatInterface::setTextSubScriptCommand(bool on)
{
    KoTextFormat format( *currentFormat() );
    if(!on)
        format.setVAlign(KoTextFormat::AlignNormal);
    else
        format.setVAlign(KoTextFormat::AlignSubScript);
    return setFormatCommand( &format, KoTextFormat::VAlign );
}

KCommand *KoTextFormatInterface::setTextSuperScriptCommand(bool on)
{
    KoTextFormat format( *currentFormat() );
    if(!on)
        format.setVAlign(KoTextFormat::AlignNormal);
    else
        format.setVAlign(KoTextFormat::AlignSuperScript);
    return setFormatCommand( &format, KoTextFormat::VAlign );
}

KCommand *KoTextFormatInterface::setDefaultFormatCommand()
{
    KoTextFormatCollection * coll = currentFormat()->parent();
    Q_ASSERT(coll);
    if(coll)
    {
        KoTextFormat * format = coll->defaultFormat();
        return setFormatCommand( format, KoTextFormat::Format );
    } else {
        kDebug() << "useless call to setDefaultFormatCommand at: " << kBacktrace() << endl;
    }
    return 0;
}

KCommand *KoTextFormatInterface::setAlignCommand(int align)
{
    KoParagLayout format( *currentParagLayoutFormat() );
    format.alignment=align;
    return setParagLayoutFormatCommand(&format,KoParagLayout::Alignment);
}

KCommand *KoTextFormatInterface::setHyphenationCommand( bool _b )
{
    KoTextFormat format( *currentFormat() );
    format.setHyphenation( _b );
    return setFormatCommand( &format, KoTextFormat::Hyphenation);
}


KCommand *KoTextFormatInterface::setShadowTextCommand( double shadowDistanceX, double shadowDistanceY, const QColor& shadowColor )
{
    KoTextFormat format( *currentFormat() );
    format.setShadow( shadowDistanceX, shadowDistanceY, shadowColor );
    return setFormatCommand( &format, KoTextFormat::ShadowText );
}

KCommand *KoTextFormatInterface::setFontAttributeCommand( KoTextFormat::AttributeStyle _att)
{
    KoTextFormat format( *currentFormat() );
    format.setAttributeFont( _att );
    return setFormatCommand( &format, KoTextFormat::Attribute );
}

KCommand *KoTextFormatInterface::setRelativeTextSizeCommand( double _size )
{
    KoTextFormat format( *currentFormat() );
    format.setRelativeTextSize( _size );
    return setFormatCommand( &format, KoTextFormat::VAlign );
}

KCommand *KoTextFormatInterface::setOffsetFromBaseLineCommand( int _offset )
{
    KoTextFormat format( *currentFormat() );
    format.setOffsetFromBaseLine( _offset );
    return setFormatCommand( &format, KoTextFormat::OffsetFromBaseLine );
}

KCommand *KoTextFormatInterface::setWordByWordCommand( bool _b )
{
    KoTextFormat format( *currentFormat() );
    format.setWordByWord( _b );
    return setFormatCommand( &format, KoTextFormat::WordByWord );
}

#if 0
void KoTextFormatInterface::setAlign(int align)
{
    KCommand *cmd = setAlignCommand( align );
    emitNewCommand( cmd );
}

void KoTextFormatInterface::setMargin(Q3StyleSheetItem::Margin m, double margin)
{
    KCommand *cmd = setMarginCommand( m, margin );
    emitNewCommand( cmd );
}

void KoTextFormatInterface::setTabList(const KoTabulatorList & tabList )
{
    KCommand *cmd = setTabListCommand( tabList );
    emitNewCommand( cmd );
}

void KoTextFormatInterface::setCounter(const KoParagCounter & counter )
{
    KCommand *cmd = setCounterCommand( counter );
    emitNewCommand( cmd );
}

void KoTextFormatInterface::setParagLayoutFormat( KoParagLayout *newLayout, int flags, int marginIndex)
{
    KCommand *cmd = setParagLayoutFormatCommand(newLayout, flags, marginIndex);
    emitNewCommand( cmd );
}
#endif

KCommand *KoTextFormatInterface::setMarginCommand(Q3StyleSheetItem::Margin m, double margin)
{
    KoParagLayout format( *currentParagLayoutFormat() );
    format.margins[m]=margin;
    return setParagLayoutFormatCommand(&format,KoParagLayout::Margins,(int)m);
}

KCommand *KoTextFormatInterface::setTabListCommand(const KoTabulatorList & tabList )
 {
    KoParagLayout format( *currentParagLayoutFormat() );
    format.setTabList(tabList);
    return setParagLayoutFormatCommand(&format,KoParagLayout::Tabulator);
}

KCommand *KoTextFormatInterface::setCounterCommand(const KoParagCounter & counter )
{
    KoParagLayout format( *currentParagLayoutFormat() );
    if(!format.counter)
        format.counter = new KoParagCounter;
    *format.counter = counter;
    return setParagLayoutFormatCommand(&format,KoParagLayout::BulletNumber);
}

KCommand *KoTextFormatInterface::setLanguageCommand(const QString &_lang)
{
    KoTextFormat format( *currentFormat() );
    format.setLanguage(_lang);
    return setFormatCommand( &format, KoTextFormat::Language );
}

KoTextDocCommand *KoTextFormatInterface::deleteTextCommand( KoTextDocument *textdoc, int id, int index, const Q3MemArray<KoTextStringChar> & str, const CustomItemsMap & customItemsMap, const Q3ValueList<KoParagLayout> & oldParagLayouts )
{
    return textdoc->deleteTextCommand( textdoc, id, index, str, customItemsMap, oldParagLayouts );
}

#include "KoTextObject.moc"
