/***************************************************************************
 *   Copyright (C) 2004 by Boudewijn Rempt                                 *
 *   boud@valdyas.org                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

/* This template is based off of the KOffice example written by Torben Weis <weis@kde.org
   It was converted to a KDevelop template by Ian Reinhart Geiser <geiseri@yahoo.com>
*/
#include "krita_view.h"
#include "krita_factory.h"
#include "krita_part.h"

#include <qpainter.h>
#include <qiconset.h>
#include <kaction.h>
#include <kstdaction.h>
#include <klocale.h>
#include <kdebug.h>

kritaView::kritaView( kritaPart* part, QWidget* parent, const char* name )
    : KoView( part, parent, name )
{
    setInstance( kritaFactory::global() );
    setXMLFile( "krita.rc" );
    KStdAction::cut(this, SLOT( cut() ), actionCollection(), "cut" );
    // Note: Prefer KStdAction::* to any custom action if possible.
    //m_cut = new KAction( i18n("&Cut"), "editcut", 0, this, SLOT( cut() ),
    //                   actionCollection(), "cut");
}

void kritaView::paintEvent( QPaintEvent* ev )
{
    QPainter painter;
    painter.begin( this );

    /// @todo Scaling

    // Let the document do the drawing
    koDocument()->paintEverything( painter, ev->rect(), FALSE, this );

    painter.end();
}

void kritaView::updateReadWrite( bool /*readwrite*/ )
{
#ifdef __GNUC__
#warning TODO
#endif
}

void kritaView::cut()
{
    kdDebug(31000) << "kritaView::cut(): CUT called" << endl;
}

#include "krita_view.moc"
