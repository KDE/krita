/* This file is part of the KDE project
   Copyright (C) 2002 Heinrich Kuettler <heinrich.kuettler@gmx.de>

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

#include <q3listbox.h>
#include <qpainter.h>
#include <Q3ComboBox>
//Added by qt3to4:
#include <Q3ValueList>
#include <Q3MemArray>

#include <kapplication.h>
#include <kcombobox.h>
#include <kglobalsettings.h>
#include <ktoolbar.h>
#include <kdebug.h>
#include <kauthorized.h>

#include "symbolaction.h"

/*
 * The items for the SymbolCombos. *
 */

KFORMULA_NAMESPACE_BEGIN

class SymbolComboItem : public Q3ListBoxItem
{
public:
    SymbolComboItem( const QString&, const QFont&, uchar, Q3ComboBox* combo );
    virtual ~SymbolComboItem();

    virtual int width( const Q3ListBox* ) const;
    virtual int height( const Q3ListBox* ) const;

protected:
    virtual void paint( QPainter *p );

private:
    Q3ComboBox *m_combo;
    QString m_name;
    QFont m_font;
    uchar m_symbol;

    static int widest;
};

int SymbolComboItem::widest = 0;

SymbolComboItem::SymbolComboItem( const QString &name, const QFont &font,
                                  uchar symbol, Q3ComboBox *combo )
    : Q3ListBoxItem( combo->listBox() ),
      m_combo( combo ),
      m_name( name ),
      m_font( font ),
      m_symbol( symbol )
{
    setText( name );
    int charWidth = QFontMetrics( m_font ).width( QChar( m_symbol ) );
    widest = qMax( widest, charWidth );
}

SymbolComboItem::~SymbolComboItem()
{
}

int SymbolComboItem::width( const Q3ListBox * /*lb*/ ) const
{
    return widest + QFontMetrics( KGlobalSettings::generalFont() ).width( text() ) + 12;
}

int SymbolComboItem::height( const Q3ListBox * /*lb*/ ) const
{
    int generalHeight = QFontMetrics( KGlobalSettings::generalFont() ).lineSpacing();
    int fontHeight = QFontMetrics( m_font ).lineSpacing();
    return qMax( generalHeight, fontHeight ) + 2;
}

void SymbolComboItem::paint( QPainter *p )
{
    p->setFont( m_font );
    QFontMetrics fm( p->fontMetrics() );
    p->drawText( 3, fm.ascent() + fm.leading() / 2,
                 QString( "%1" ).arg( QChar( m_symbol ) ) );

    p->setFont( KGlobalSettings::generalFont() );
    fm = p->fontMetrics();
    p->drawText( widest + 6, height( m_combo->listBox() ) / 2 + fm.strikeOutPos(), m_name );
}

/*
 * The symbol action *
 */
SymbolAction::SymbolAction( KActionCollection* parent, const char* name )
    : KSelectAction( parent, name )
{
    setEditable( FALSE );
}

SymbolAction::SymbolAction( const QString& text, const KShortcut& cut,
                            const QObject* receiver, const char* slot,
                            KActionCollection* parent, const char* name )
    : KSelectAction( text, cut, receiver, slot, parent, name )
{
    setEditable( FALSE );
}

int SymbolAction::plug( QWidget* w, int index )
{
    if (!KAuthorized::authorizeKAction(name()))
        return -1;
    if ( w->inherits( "KToolBar" ) )
    {
#warning "kde4: port it"			
#if 0
			KToolBar* bar = static_cast<KToolBar*>( w );
        int id_ = KAction::getToolButtonID();
        KComboBox *cb = new KComboBox( bar );
        connect( cb, SIGNAL( activated( const QString & ) ),
                 SLOT( slotActivated( const QString & ) ) );
        cb->setEnabled( isEnabled() );
        bar->insertWidget( id_, comboWidth(), cb, index );
        cb->setMinimumWidth( cb->sizeHint().width() );

        addContainer( bar, id_ );

        connect( bar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );

        updateItems( containerCount() - 1 );

        return containerCount() - 1;
#endif
		return 0;
    }
    else return KSelectAction::plug( w, index );	
}

void SymbolAction::setSymbols( const QStringList &names, const Q3ValueList<QFont>& fonts,
                               const Q3MemArray<uchar>& chars )
{
    m_fonts = fonts;
    m_chars = chars;
    setItems( names );

    int len = containerCount();
    for ( int i = 0; i < len; ++i )
        updateItems( i );
}

void SymbolAction::updateItems( int id )
{
    QWidget *w = container( id );
    if ( w->inherits( "KToolBar" ) ) {
#warning "kde4: port it"			
#if 0			
        QWidget *r = static_cast<KToolBar*>( w )->getWidget( itemId( id ) );
        if ( r->inherits( "Q3ComboBox" ) ) {
            Q3ComboBox *cb = static_cast<Q3ComboBox*>( r );
            cb->clear();

            for( uint i = 0; i < items().count(); ++i ) {
                new SymbolComboItem( *items().at( i ), *m_fonts.at( i ),
                                     m_chars.at( i ), cb );
            }
            cb->setMinimumWidth( cb->sizeHint().width() );
        }
#endif		
    }
}

KFORMULA_NAMESPACE_END
