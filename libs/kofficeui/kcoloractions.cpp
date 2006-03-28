/* This file is part of the KDE libraries
    Copyright (C) 2000 Reginald Stadlbauer <reggie@kde.org>
    Copyright (C) 2002 Werner Trobin <trobin@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include <kcoloractions.h>

#include <q3popupmenu.h>
#include <kauthorized.h>
#include <qtooltip.h>
//Added by qt3to4:
#include <QPixmap>
#include <Q3CString>

#include <kapplication.h>
#include <ktoolbar.h>

#include <kdebug.h>

KColorAction::KColorAction( const QString& text, int accel,
			    QObject* parent, const char* name )
    : KAction( text, accel, parent, name )
{
    typ = TextColor;
    init();
}

KColorAction::KColorAction( const QString& text, int accel,
			    QObject* receiver, const char* slot, QObject* parent,
			    const char* name )
    : KAction( text, accel, receiver, slot, parent, name )
{
    typ = TextColor;
    init();
}

KColorAction::KColorAction( const QString& text, Type type, int accel,
			    QObject* parent, const char* name )
    : KAction( text, accel, parent, name )
{
    typ = type;
    init();
}

KColorAction::KColorAction( const QString& text, Type type, int accel,
			    QObject* receiver, const char* slot, QObject* parent,
			    const char* name )
    : KAction( text, accel, receiver, slot, parent, name )
{
    typ = type;
    init();
}

KColorAction::KColorAction( QObject* parent, const char* name )
    : KAction( parent, name )
{
    typ = TextColor;
    init();
}

void KColorAction::setColor( const QColor &c )
{
    if ( c == col )
	return;

    col = c;
    createPixmap();
}

QColor KColorAction::color() const
{
    return col;
}

void KColorAction::setType( Type t )
{
    if ( t == typ )
	return;

    typ = t;
    createPixmap();
}

KColorAction::Type KColorAction::type() const
{
    return typ;
}

void KColorAction::init()
{
    col = Qt::black;
    createPixmap();
}

void KColorAction::createPixmap()
{
    int r, g, b;
    Q3CString pix;
    Q3CString line;

    col.rgb( &r, &g, &b );

    pix = "/* XPM */\n";

    pix += "static char * text_xpm[] = {\n";

    switch ( typ ) {
      case TextColor: {
        pix += "\"20 20 11 1\",\n";
        pix += "\"h c #c0c000\",\n";
        pix += "\"g c #808000\",\n";
        pix += "\"f c #c0c0ff\",\n";
        pix += "\"a c #000000\",\n";
        pix += "\"d c #ff8000\",\n";
        pix += "\". c none\",\n";
        pix += "\"e c #0000c0\",\n";
        pix += "\"i c #ffff00\",\n";
        line.sprintf( "\"# c #%02X%02X%02X \",\n", r, g, b );
        pix += line.copy();
        pix += "\"b c #c00000\",\n";
        pix += "\"c c #ff0000\",\n";
        pix += "\"....................\",\n";
        pix += "\"....................\",\n";
        pix += "\"....................\",\n";
        pix += "\"........#...........\",\n";
        pix += "\"........#a..........\",\n";
        pix += "\".......###..........\",\n";
        pix += "\".......###a.........\",\n";
        pix += "\"......##aa#.........\",\n";
        pix += "\"......##a.#a........\",\n";
        pix += "\".....##a...#........\",\n";
        pix += "\".....#######a.......\",\n";
        pix += "\"....##aaaaaa#.......\",\n";
        pix += "\"....##a.....aaaaaaaa\",\n";
        pix += "\"...####....#abbccdda\",\n";
        pix += "\"....aaaa....abbccdda\",\n";
        pix += "\"............aee##ffa\",\n";
        pix += "\"............aee##ffa\",\n";
        pix += "\"............agghhiia\",\n";
        pix += "\"............agghhiia\",\n";
        pix += "\"............aaaaaaaa\"};\n";
      } break;
      case FrameColor: {
	pix += "\" 20 20 3 1 \",\n";

	pix += "\"  c none \",\n";
	pix += "\"+ c white \",\n";
	line.sprintf( "\". c #%02X%02X%02X \",\n", r, g, b );
	pix += line.copy();

	pix += "\"                     \",\n";
	pix += "\"                     \",\n";
	pix += "\"  ................  \",\n";
	pix += "\"  ................  \",\n";
	pix += "\"  ................  \",\n";
	pix += "\"  ...++++++++++...  \",\n";
	pix += "\"  ...++++++++++...  \",\n";
	pix += "\"  ...++++++++++...  \",\n";
	pix += "\"  ...++++++++++...  \",\n";
	pix += "\"  ...++++++++++...  \",\n";
	pix += "\"  ...++++++++++...  \",\n";
	pix += "\"  ...++++++++++...  \",\n";
	pix += "\"  ...++++++++++...  \",\n";
	pix += "\"  ...++++++++++...  \",\n";
	pix += "\"  ...++++++++++...  \",\n";
	pix += "\"  ................  \",\n";
	pix += "\"  ................  \",\n";
	pix += "\"  ................  \",\n";
	pix += "\"                     \",\n";
	pix += "\"                     \";\n";
      } break;
      case BackgroundColor: {
        pix += "\" 20 20 3 1 \",\n";

        pix += "\"  c none \",\n";
        pix += "\". c red \",\n";
        line.sprintf( "\"+ c #%02X%02X%02X \",\n", r, g, b );
        pix += line.copy();

        pix += "\"                     \",\n";
        pix += "\"                     \",\n";
        pix += "\"  ................  \",\n";
        pix += "\"  ................  \",\n";
        pix += "\"  ..++++++++++++..  \",\n";
        pix += "\"  ..++++++++++++..  \",\n";
        pix += "\"  ..++++++++++++..  \",\n";
        pix += "\"  ..++++++++++++..  \",\n";
        pix += "\"  ..++++++++++++..  \",\n";
        pix += "\"  ..++++++++++++..  \",\n";
        pix += "\"  ..++++++++++++..  \",\n";
        pix += "\"  ..++++++++++++..  \",\n";
        pix += "\"  ..++++++++++++..  \",\n";
        pix += "\"  ..++++++++++++..  \",\n";
        pix += "\"  ..++++++++++++..  \",\n";
        pix += "\"  ..++++++++++++..  \",\n";
        pix += "\"  ................  \",\n";
        pix += "\"  ................  \",\n";
        pix += "\"                     \",\n";
        pix += "\"                     \";\n";
      } break;
    }

    QPixmap pixmap( pix );
    setIconSet( QIcon( pixmap ) );
}


KSelectColorAction::KSelectColorAction( const QString& text, Type type,
                                        const QObject* receiver, const char* slot,
                                        KActionCollection* parent, const char* name ) :
    KAction( text, KShortcut(), receiver, slot, parent, name ), m_type( type ),
    m_color( Qt::black )
{
}

KSelectColorAction::~KSelectColorAction()
{
}

int KSelectColorAction::plug( QWidget* w, int index )
{
    if (w == 0) {
	kWarning() << "KSelectColorAction::plug called with 0 argument\n";
 	return -1;
    }
    if (!KAuthorized::authorizeKAction(name()))
        return -1;

    if ( w->inherits("QPopupMenu") )
    {
        Q3PopupMenu* menu = static_cast<Q3PopupMenu*>( w );
        int id;

        if ( hasIcon() )
        {
            /* ###### CHECK: We're not allowed to specify the instance in iconSet()
            KInstance *instance;
            if ( parentCollection() )
                instance = parentCollection()->instance();
            else
                instance = KGlobal::instance();
            */
            id = menu->insertItem( iconSet( K3Icon::Small, 0 ), text(), this,//dsweet
                                   SLOT( slotActivated() ), 0, -1, index );
        }
        else
            id = menu->insertItem( text(), this, SLOT( slotActivated() ),  //dsweet
                                   0, -1, index );

        updateShortcut( menu, id );

        // call setItemEnabled only if the item really should be disabled,
        // because that method is slow and the item is per default enabled
        if ( !isEnabled() )
            menu->setItemEnabled( id, false );

        if ( !whatsThis().isEmpty() )
            menu->setWhatsThis( id, whatsThisWithIcon() );

        addContainer( menu, id );
        connect( menu, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );

        if ( parentCollection() )
            parentCollection()->connectHighlight( menu, this );

        return containerCount() - 1;
    }
    else if ( w->inherits( "KToolBar" ) )
    {
        KToolBar *bar = static_cast<KToolBar *>( w );

        int id_ = getToolButtonID();
        KInstance *instance;
        if ( parentCollection() )
            instance = parentCollection()->instance();
        else
            instance = KGlobal::instance();

        if ( icon().isEmpty() ) // old code using QIcon directly
        {
            bar->insertButton( iconSet( K3Icon::Small ).pixmap(), id_, SIGNAL( clicked() ), this,
                               SLOT( slotActivated() ),
                               isEnabled(), plainText(), index );
        }
        else
            bar->insertButton( icon(), id_, SIGNAL( clicked() ), this,
                               SLOT( slotActivated() ),
                               isEnabled(), plainText(), index, instance );

        bar->getButton( id_ )->setName( Q3CString("toolbutton_")+name() );

        if ( !whatsThis().isEmpty() )
            bar->getButton(id_)->setWhatsThis( whatsThisWithIcon() );

        if ( !toolTip().isEmpty() )
            bar->getButton(id_)->setToolTip( toolTip() );

        addContainer( bar, id_ );

        connect( bar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );

        if ( parentCollection() )
            parentCollection()->connectHighlight( bar, this );

        return containerCount() - 1;
    }

    return -1;
}

QColor KSelectColorAction::color() const
{
    return m_color;
}

KSelectColorAction::Type KSelectColorAction::type() const
{
    return m_type;
}

void KSelectColorAction::setColor( const QColor &/*c*/ )
{
}

void KSelectColorAction::setType( Type /*t*/ )
{
}

QString KSelectColorAction::whatsThisWithIcon() const
{
    QString text = whatsThis();
    if (!icon().isEmpty())
      return QString::fromLatin1("<img source=\"small|%1\"> %2").arg(icon()).arg(text);
    return text;
}

#include <kcoloractions.moc>
#include <kauthorized.h>
