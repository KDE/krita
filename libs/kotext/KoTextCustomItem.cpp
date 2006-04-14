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

#include "KoRichText.h"
#include "KoTextFormat.h"
#include "KoTextParag.h"
#include "KoTextZoomHandler.h"
#include "KoTextDocument.h"
#include <kdebug.h>
#include <kcommand.h>


//void KoTextCustomItem::setPainter( QPainter*, bool adjust ){ if ( adjust ) width = 0; }
//void KoTextCustomItem::setPainter( QPainter*, bool ){ resize(); } // changed for kotext

bool KoTextCustomItem::enter( KoTextCursor *, KoTextDocument *&doc, KoTextParag *&parag, int &idx, int &ox, int &oy, bool atEnd )
{
    doc = doc; parag = parag; idx = idx; ox = ox; oy = oy; Q_UNUSED( atEnd ) return TRUE;

}
bool KoTextCustomItem::enterAt( KoTextCursor *, KoTextDocument *&doc, KoTextParag *&parag, int &idx, int &ox, int &oy, const QPoint & )
{
    doc = doc; parag = parag; idx = idx; ox = ox; oy = oy; return TRUE;
}
bool KoTextCustomItem::next( KoTextCursor *, KoTextDocument *&doc, KoTextParag *&parag, int &idx, int &ox, int &oy )
{
    doc = doc; parag = parag; idx = idx; ox = ox; oy = oy; return TRUE;
}
bool KoTextCustomItem::prev( KoTextCursor *, KoTextDocument *&doc, KoTextParag *&parag, int &idx, int &ox, int &oy )
{
    doc = doc; parag = parag; idx = idx; ox = ox; oy = oy; return TRUE;
}
bool KoTextCustomItem::down( KoTextCursor *, KoTextDocument *&doc, KoTextParag *&parag, int &idx, int &ox, int &oy )
{
    doc = doc; parag = parag; idx = idx; ox = ox; oy = oy; return TRUE;
}
bool KoTextCustomItem::up( KoTextCursor *, KoTextDocument *&doc, KoTextParag *&parag, int &idx, int &ox, int &oy )
{
    doc = doc; parag = parag; idx = idx; ox = ox; oy = oy; return TRUE;
}

int KoTextCustomItem::index() const
{
    Q_ASSERT( paragraph() );
    KoTextParag * parag = paragraph();
    return parag->findCustomItem( this );
}

KoTextFormat * KoTextCustomItem::format() const
{
    KoTextParag * parag = paragraph();
    //kDebug(32500) << "KoTextCustomItem::format index=" << index() << " format=" << parag->at( index() )->format() << endl;
    return parag->at( index() )->format();
}


void KoTextCustomItem::draw(QPainter* p, int _x, int _y, int cx, int cy, int cw, int ch, const QColorGroup& cg, bool selected )
{
    KoTextZoomHandler *zh=textDocument()->paintingZoomHandler();
    //kDebug(32500)<<" x :"<<_x<<" y :"<<_y<<" cx :"<<cx<<" cy :"<<cy<<" ch :"<<ch<<" cw :"<<cw<<endl;

    // Calculate index only once
    // Hmm, should pass it to drawCustomItem...
    int charIndex = index();
    KoTextStringChar* stringChar = paragraph()->at( charIndex );

    // Convert x, y, cx, cy, cw and ch from Layout Units to pixels.
    int x = zh->layoutUnitToPixelX(_x) /*+ stringChar->pixelxadj*/;
    int y = zh->layoutUnitToPixelY(_y);
    cx = zh->layoutUnitToPixelX(cx);
    cy = zh->layoutUnitToPixelY(cy);
    cw = zh->layoutUnitToPixelX(_x,cw);
    ch = zh->layoutUnitToPixelY(_y,ch);
    int wpix = zh->layoutUnitToPixelX(_x,width);
    int hpix = zh->layoutUnitToPixelX(_y,height);
    //kDebug(32500)<<"After  x :"<<x<<" y :"<<y<<" cx :"<<cx<<" cy :"<<cy<<" ch :"<<ch<<" cw :"<<cw<<endl;
    int ascentpix = zh->layoutUnitToPixelY( _y, ascent() );

    KoTextFormat * fmt = stringChar->format();

    //bool forPrint = ( p->device()->devType() == QInternal::Printer );
    p->setFont( fmt->screenFont( zh ) );

    int offset=0;
    if ( fmt->vAlign() == KoTextFormat::AlignSuperScript )
        offset = -( hpix - p->fontMetrics().height() );

    if ( fmt->shadowDistanceX() != 0 || fmt->shadowDistanceY() != 0 ) {
        int sx = fmt->shadowX( zh );
        int sy = fmt->shadowY( zh );
        if ( sx != 0 || sy != 0 )
        {
            p->save();
            p->translate( sx, sy );
            drawCustomItem(p, x, y, wpix, hpix, ascentpix, cx, cy, cw, ch, cg, selected, offset, true);
            p->restore();
        }
    }
    drawCustomItem(p, x, y, wpix, hpix, ascentpix, cx, cy, cw, ch, cg, selected, offset,  false);
}

////////////////

void CustomItemsMap::insertItems( const KoTextCursor & startCursor, int size )
{
    if ( isEmpty() )
        return;

    KoTextCursor cursor( startCursor );
    for ( int i = 0; i < size; ++i )
    {
        CustomItemsMap::Iterator it = find( i );
        if ( it != end() )
        {
            kDebug(32500) << "CustomItemsMap::insertItems setting custom item " << it.value() << endl;
            cursor.parag()->setCustomItem( cursor.index(), it.value(), 0 );
            it.value()->setDeleted( false );
        }
        cursor.gotoRight();
    }
}

void CustomItemsMap::deleteAll( KMacroCommand *macroCmd )
{
    KoTextCustomItem* item;	
    foreach( item, *this )
    {
        KCommand * itemCmd = item->deleteCommand();
        if ( itemCmd && macroCmd )
        {
            macroCmd->addCommand( itemCmd );
            itemCmd->execute(); // the item-specific delete stuff hasn't been done
        }
        item->setDeleted( true );
    }
}
