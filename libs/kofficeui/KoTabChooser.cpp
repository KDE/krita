/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

// Description: Tabulator chooser (header)

/******************************************************************/

#include <KoTabChooser.h>
#include <qpainter.h>
#include <q3popupmenu.h>
#include <qcursor.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <Q3Frame>

#include <klocale.h>


class KoTabChooserPrivate {
public:
    KoTabChooserPrivate() {
    }
    ~KoTabChooserPrivate() {}

    bool m_bReadWrite;
};

/******************************************************************/
/* Class: KoTabChooser						  */
/******************************************************************/

/*================================================================*/
KoTabChooser::KoTabChooser( QWidget *parent, int _flags )
    : Q3Frame( parent, "" )
{
    setFrameStyle( Q3Frame::StyledPanel );
    flags = _flags;
    d=new KoTabChooserPrivate();

    d->m_bReadWrite=true;

    currType = 0;

    if ( flags & TAB_DEC_PNT ) currType = TAB_DEC_PNT;
    if ( flags & TAB_CENTER ) currType = TAB_CENTER;
    if ( flags & TAB_RIGHT ) currType = TAB_RIGHT;
    if ( flags & TAB_LEFT ) currType = TAB_LEFT;

    setupMenu();
}

/*================================================================*/
void KoTabChooser::mousePressEvent( QMouseEvent *e )
{
    if ( currType == 0 ) return;

    if( !d->m_bReadWrite)
        return;

    switch ( e->button() ) {
    case Qt::LeftButton: case Qt::MidButton: {
	switch ( currType ) {
	case TAB_LEFT: {
	    if ( flags & TAB_CENTER ) currType = TAB_CENTER;
	    else if ( flags & TAB_RIGHT ) currType = TAB_RIGHT;
	    else if ( flags & TAB_DEC_PNT ) currType = TAB_DEC_PNT;
	} break;
	case TAB_RIGHT: {
	    if ( flags & TAB_DEC_PNT ) currType = TAB_DEC_PNT;
	    else if ( flags & TAB_LEFT ) currType = TAB_LEFT;
	    else if ( flags & TAB_CENTER ) currType = TAB_CENTER;
	} break;
	case TAB_CENTER: {
	    if ( flags & TAB_RIGHT ) currType = TAB_RIGHT;
	    else if ( flags & TAB_DEC_PNT ) currType = TAB_DEC_PNT;
	    else if ( flags & TAB_LEFT ) currType = TAB_LEFT;
	} break;
	case TAB_DEC_PNT: {
	    if ( flags & TAB_LEFT ) currType = TAB_LEFT;
	    else if ( flags & TAB_CENTER ) currType = TAB_CENTER;
	    else if ( flags & TAB_RIGHT ) currType = TAB_RIGHT;
	} break;
	}
	repaint( true );
    } break;
    case Qt::RightButton: {
	QPoint pnt( QCursor::pos() );

	rb_menu->setItemChecked( mLeft, false );
	rb_menu->setItemChecked( mCenter, false );
	rb_menu->setItemChecked( mRight, false );
	rb_menu->setItemChecked( mDecPoint, false );

	switch ( currType ) {
	case TAB_LEFT: rb_menu->setItemChecked( mLeft, true );
	    break;
	case TAB_CENTER: rb_menu->setItemChecked( mCenter, true );
	    break;
	case TAB_RIGHT: rb_menu->setItemChecked( mRight, true );
	    break;
	case TAB_DEC_PNT: rb_menu->setItemChecked( mDecPoint, true );
	    break;
	}

	rb_menu->popup( pnt );
    } break;
    default: break;
    }
}

/*================================================================*/
void KoTabChooser::drawContents( QPainter *painter )
{
    if ( currType == 0 ) return;

    painter->setPen( QPen( Qt::black, 2, Qt::SolidLine ) );

    switch ( currType ) {
    case TAB_LEFT: {
	painter->drawLine( 4, height() - 4, width() - 4, height() - 4 );
	painter->drawLine( 5, 4, 5, height() - 4 );
    } break;
    case TAB_CENTER: {
	painter->drawLine( 4, height() - 4, width() - 4, height() - 4 );
	painter->drawLine( width() / 2, 4, width() / 2, height() - 4 );
    } break;
    case TAB_RIGHT: {
	painter->drawLine( 4, height() - 4, width() - 4, height() - 4 );
	painter->drawLine( width() - 5, 4, width() - 5, height() - 4 );
    } break;
    case TAB_DEC_PNT: {
	painter->drawLine( 4, height() - 4, width() - 4, height() - 4 );
	painter->drawLine( width() / 2, 4, width() / 2, height() - 4 );
	painter->fillRect( width() / 2 + 2, height() - 9, 3, 3, Qt::black );
    } break;
    default: break;
    }
}

/*================================================================*/
void KoTabChooser::setupMenu()
{
    rb_menu = new Q3PopupMenu();
    Q_CHECK_PTR( rb_menu );
    mLeft = rb_menu->insertItem( i18n( "Tabulator &Left" ), this, SLOT( rbLeft() ) );
    mCenter = rb_menu->insertItem( i18n( "Tabulator &Center" ), this, SLOT( rbCenter() ) );
    mRight = rb_menu->insertItem( i18n( "Tabulator &Right" ), this, SLOT( rbRight() ) );
    mDecPoint =  rb_menu->insertItem( i18n( "Tabulator &Decimal Point" ), this, SLOT( rbDecPoint() ) );
    rb_menu->setCheckable( false );
}

KoTabChooser::~KoTabChooser() {
    delete rb_menu;
    delete d;
}

void KoTabChooser::setReadWrite(bool _readWrite)
{
    d->m_bReadWrite=_readWrite;
}

#include <KoTabChooser.moc>
