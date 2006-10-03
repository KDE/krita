/* This file is part of the KDE project
   Copyright (C) 2001 David Faure <faure@kde.org>

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

#include "KoTextCommand.h"
#include "KoTextObject.h"
#include "KoTextParag.h"
#include "KoVariable.h"
#include <kdebug.h>
#include <klocale.h>
//Added by qt3to4:
#include <Q3ValueList>
#include <Q3MemArray>

// This is automatically called by KCommandHistory's redo action when redo is activated
void KoTextCommand::execute()
{
    m_textobj->redo();
}

// This is automatically called by KCommandHistory's undo action when undo is activated
void KoTextCommand::unexecute()
{
    m_textobj->undo();
}

KoTextDeleteCommand::KoTextDeleteCommand(
    KoTextDocument *d, int i, int idx, const Q3MemArray<KoTextStringChar> &str,
    const CustomItemsMap & customItemsMap,
    const Q3ValueList<KoParagLayout> &oldParagLayouts )
    : KoTextDocDeleteCommand( d, i, idx, str ),
      m_oldParagLayouts( oldParagLayouts ),
      m_customItemsMap( customItemsMap )
{
    Q_ASSERT( id >= 0 );
}

KoTextCursor * KoTextDeleteCommand::execute( KoTextCursor *c )
{
    KoTextParag *s = doc ? doc->paragAt( id ) : parag;
    if ( !s ) {
        kWarning() << "can't locate parag at " << id << ", last parag: " << doc->lastParag()->paragId() << endl;
        return 0;
    }
    cursor.setParag( s );
    cursor.setIndex( index );
    int len = text.size();
    // Detach from custom items. They are already in the map, and we don't
    // want them to be deleted
    for ( int i = 0; i < len; ++i )
    {
        KoTextStringChar * ch = cursor.parag()->at( cursor.index() );
        if ( ch->isCustom() )
        {
            ch->customItem()->setDeleted( true );
            cursor.parag()->removeCustomItem(cursor.index());
        }
        cursor.gotoRight();
    }

    return KoTextDocDeleteCommand::execute(c);
}

KoTextCursor * KoTextDeleteCommand::unexecute( KoTextCursor *c )
{
    // Let QRichText re-create the text and formatting
    KoTextCursor * cr = KoTextDocDeleteCommand::unexecute(c);

    KoTextParag *s = doc ? doc->paragAt( id ) : parag;
    if ( !s ) {
        kWarning() << "can't locate parag at " << id << ", last parag: " << (doc ? doc->lastParag()->paragId() : -1) << endl;
        return 0;
    }
    cursor.setParag( s );
    cursor.setIndex( index );
    // Set any custom item that we had
    m_customItemsMap.insertItems( cursor, text.size() );

    // Now restore the parag layouts (i.e. libkotext specific stuff)
    Q3ValueList<KoParagLayout>::Iterator lit = m_oldParagLayouts.begin();
    kDebug(32500) << "KoTextDeleteCommand::unexecute " << m_oldParagLayouts.count() << " parag layouts. First parag=" << s->paragId() << endl;
    Q_ASSERT( id == s->paragId() );
    KoTextParag *p = s;
    while ( p ) {
        if ( lit != m_oldParagLayouts.end() )
        {
            kDebug(32500) << "KoTextDeleteCommand::unexecute applying paraglayout to parag " << p->paragId() << endl;
            p->setParagLayout( *lit );
        }
        else
            break;
        //if ( s == cr->parag() )
        //    break;
        p = p->next();
        ++lit;
    }
    return cr;
}

KoTextParagCommand::KoTextParagCommand( KoTextDocument *d, int fParag, int lParag,
                                        const Q3ValueList<KoParagLayout> &oldParagLayouts,
                                        KoParagLayout newParagLayout,
                                        int flags,
                                        Q3StyleSheetItem::Margin margin, bool borderOutline )
    : KoTextDocCommand( d ), firstParag( fParag ), lastParag( lParag ), m_oldParagLayouts( oldParagLayouts ),
      m_newParagLayout( newParagLayout ), m_flags( flags ), m_margin( margin ), m_borderOutline( borderOutline )
{
    Q_ASSERT( fParag >= 0 );
    Q_ASSERT( lParag >= 0 );
}

KoTextCursor * KoTextParagCommand::execute( KoTextCursor *c )
{
    //kDebug(32500) << "KoTextParagCommand::execute" << endl;
    KoTextParag *p = doc->paragAt( firstParag );
    if ( !p )
    {
        kWarning() << "KoTextParagCommand::execute paragraph " << firstParag << "not found" << endl;
        return c;
    }
    while ( p ) {
        if ( ( m_flags & KoParagLayout::Margins ) && m_margin != (Q3StyleSheetItem::Margin)-1 ) // all
            p->setMargin( static_cast<Q3StyleSheetItem::Margin>(m_margin), m_newParagLayout.margins[m_margin] );
        else
        {
            p->setParagLayout( m_newParagLayout, m_flags );
            if ( (m_flags & KoParagLayout::Borders) && m_borderOutline)
            {
                KoBorder tmpBorder;
                tmpBorder.setPenWidth(0);
                p->setTopBorder(tmpBorder);
                p->setBottomBorder(tmpBorder);
            }
        }
        if ( p->paragId() == lastParag )
            break;
        p = p->next();
    }
    if ( (m_flags & KoParagLayout::Borders) && m_borderOutline)
    {
        p->setBottomBorder( m_newParagLayout.bottomBorder);
        doc->paragAt( firstParag )->setTopBorder( m_newParagLayout.topBorder);
    }

    //kDebug(32500) << "KoTextParagCommand::execute done" << endl;
    // Set cursor to end of selection. Like in KoTextFormatCommand::[un]execute...
    c->setParag( p );
    c->setIndex( p->length()-1 );
    return c;
}

KoTextCursor * KoTextParagCommand::unexecute( KoTextCursor *c )
{
    kDebug(32500) << "KoTextParagCommand::unexecute" << endl;
    KoTextParag *p = doc->paragAt( firstParag );
    if ( !p )
    {
        kDebug(32500) << "KoTextParagCommand::unexecute paragraph " << firstParag << "not found" << endl;
        return c;
    }
    Q3ValueList<KoParagLayout>::Iterator lit = m_oldParagLayouts.begin();
    while ( p ) {
        if ( lit == m_oldParagLayouts.end() )
        {
            kDebug(32500) << "KoTextParagCommand::unexecute m_oldParagLayouts not big enough!" << endl;
            break;
        }
        if ( m_flags & KoParagLayout::Margins && m_margin != (Q3StyleSheetItem::Margin)-1 ) // just one
            p->setMargin( static_cast<Q3StyleSheetItem::Margin>(m_margin), (*lit).margins[m_margin] );
        else
        {
            p->setParagLayout( *lit, m_flags );
        }
        if ( p->paragId() == lastParag )
            break;
        p = p->next();
        ++lit;
    }
    // Set cursor to end of selection. Like in KoTextFormatCommand::[un]execute...
    c->setParag( p );
    c->setIndex( p->length()-1 );
    return c;
}

//////////

KoParagFormatCommand::KoParagFormatCommand( KoTextDocument *d, int fParag, int lParag,
                                                          const Q3ValueList<KoTextFormat *> &oldFormats,
                                                          KoTextFormat * newFormat )
    : KoTextDocCommand( d ), firstParag( fParag ), lastParag( lParag ), m_oldFormats( oldFormats ),
      m_newFormat( newFormat )
{
    Q3ValueList<KoTextFormat *>::Iterator lit = m_oldFormats.begin();
    for ( ; lit != m_oldFormats.end() ; ++lit )
        (*lit)->addRef();
}

KoParagFormatCommand::~KoParagFormatCommand()
{
    Q3ValueList<KoTextFormat *>::Iterator lit = m_oldFormats.begin();
    for ( ; lit != m_oldFormats.end() ; ++lit )
        (*lit)->removeRef();
}

KoTextCursor * KoParagFormatCommand::execute( KoTextCursor *c )
{
    KoTextParag *p = doc->paragAt( firstParag );
    if ( !p )
    {
        kDebug(32500) << "KoTextParagCommand::execute paragraph " << firstParag << "not found" << endl;
        return c;
    }
    while ( p ) {
        p->setFormat( m_newFormat );
        p->invalidate(0);
        if ( p->paragId() == lastParag )
            break;
        p = p->next();
    }
    return c;
}

KoTextCursor * KoParagFormatCommand::unexecute( KoTextCursor *c )
{
    kDebug(32500) << "KoParagFormatCommand::unexecute" << endl;
    KoTextParag *p = doc->paragAt( firstParag );
    if ( !p )
    {
        kDebug(32500) << "KoParagFormatCommand::unexecute paragraph " << firstParag << "not found" << endl;
        return c;
    }
    Q3ValueList<KoTextFormat *>::Iterator lit = m_oldFormats.begin();
    while ( p ) {
        if ( lit == m_oldFormats.end() )
        {
            kDebug(32500) << "KoParagFormatCommand::unexecute m_oldFormats not big enough!" << endl;
            break;
        }
        p->setFormat( (*lit) );
        if ( p->paragId() == lastParag )
            break;
        p = p->next();
        ++lit;
    }
    return c;
}

KoTextFormatCommand::KoTextFormatCommand(KoTextDocument *d, int sid, int sidx, int eid, int eidx, const Q3MemArray<KoTextStringChar> &old, const KoTextFormat *f, int fl )
    : KoTextDocFormatCommand(d, sid, sidx, eid, eidx, old, f, fl)
{
}


KoTextFormatCommand::~KoTextFormatCommand()
{
}

void KoTextFormatCommand::resizeCustomItems()
{
    KoTextParag *sp = doc->paragAt( startId );
    KoTextParag *ep = doc->paragAt( endId );
    if ( !sp || !ep )
        return;

    KoTextCursor start( doc );
    start.setParag( sp );
    start.setIndex( startIndex );
    KoTextCursor end( doc );
    end.setParag( ep );
    end.setIndex( endIndex );

    doc->setSelectionStart( KoTextDocument::Temp, &start );
    doc->setSelectionEnd( KoTextDocument::Temp, &end );

    // TODO use the visitor pattern (some 'ResizeCustomItemVisitor')

    if ( start.parag() == end.parag() )
    {
        QString text = start.parag()->string()->toString().mid( start.index(), end.index() - start.index() );
        for ( int i = start.index(); i < end.index(); ++i )
        {
            if( start.parag()->at(i)->isCustom())
            {
                start.parag()->at(i)->customItem()->resize();
            }
        }
    }
    else
    {
        int i;
        QString text = start.parag()->string()->toString().mid( start.index(), start.parag()->length() - 1 - start.index() );
        for ( i = start.index(); i < start.parag()->length(); ++i )
            if( start.parag()->at(i)->isCustom())
            {
                start.parag()->at(i)->customItem()->resize();
            }

        KoTextParag *p = start.parag()->next();
        while ( p && p != end.parag() )
        {
            text = p->string()->toString().left( p->length() - 1 );
            for ( i = 0; i < p->length(); ++i )
            {
               if( p->at(i)->isCustom())
               {
                   p->at(i)->customItem()->resize();
               }
            }
            p = p->next();
        }
        text = end.parag()->string()->toString().left( end.index() );
        for ( i = 0; i < end.index(); ++i )
        {
            if( end.parag()->at(i)->isCustom())
            {
                end.parag()->at(i)->customItem()->resize();
            }
        }
    }
}

KoTextCursor *KoTextFormatCommand::execute( KoTextCursor *c )
{
    c = KoTextDocFormatCommand::execute( c );
    resizeCustomItems();
    return c;
}

KoTextCursor *KoTextFormatCommand::unexecute( KoTextCursor *c )
{
    kDebug(32500) << "KoTextFormatCommand::unexecute c:" << c << " index:" << c->index() << endl;
    c = KoTextDocFormatCommand::unexecute( c );
    kDebug(32500) << "KoTextFormatCommand::unexecute after KoTextFormatCommand c:" << c << " index:" << c->index() << endl;
    resizeCustomItems();
    return c;
}

////

KoChangeVariableSubType::KoChangeVariableSubType(
                        short int _oldValue, short int _newValue,
                        KoVariable *var):
    KCommand(),
    m_newValue(_newValue),
    m_oldValue(_oldValue),
    m_var(var)
{
}

void KoChangeVariableSubType::execute()
{
    Q_ASSERT(m_var);
    m_var->setVariableSubType(m_newValue);
    m_var->recalcAndRepaint();
}

void KoChangeVariableSubType::unexecute()
{
    Q_ASSERT(m_var);
    m_var->setVariableSubType(m_oldValue);
    m_var->recalcAndRepaint();
}

QString KoChangeVariableSubType::name() const
{
    return i18n( "Change Variable Subtype" );
}

////

KoChangeVariableFormatProperties::KoChangeVariableFormatProperties(
    const QString &_oldValue, const QString &_newValue,
    KoVariable *var)
    : KCommand(),
      m_newValue(_newValue),
      m_oldValue(_oldValue),
      m_var(var)
{
}

void KoChangeVariableFormatProperties::execute()
{
    Q_ASSERT(m_var);
    // Wrong! m_var->variableFormat()->setFormatProperties( m_newValue );
    KoVariableFormatCollection* coll = m_var->variableColl()->formatCollection();
    m_var->setVariableFormat( coll->format( m_var->variableFormat()->getKey( m_newValue ) ) );
    m_var->recalcAndRepaint();
}

void KoChangeVariableFormatProperties::unexecute()
{
    Q_ASSERT(m_var);
    // Wrong! m_var->variableFormat()->setFormatProperties( m_oldValue );
    KoVariableFormatCollection* coll = m_var->variableColl()->formatCollection();
    m_var->setVariableFormat( coll->format( m_var->variableFormat()->getKey( m_oldValue ) ) );
    m_var->recalcAndRepaint();
}

QString KoChangeVariableFormatProperties::name() const
{
    return i18n( "Change Variable Format" );
}
