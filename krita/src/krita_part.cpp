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

#include "krita_part.h"
#include "krita_view.h"

#include <qpainter.h>

kritaPart::kritaPart( QWidget *parentWidget, const char *widgetName, QObject* parent, const char* name, bool singleViewMode )
    : KoDocument( parentWidget, widgetName, parent, name, singleViewMode )
{
}

bool kritaPart::initDoc()
{
    // If nothing is loaded, do initialize here
    return TRUE;
}

KoView* kritaPart::createViewInstance( QWidget* parent, const char* name )
{
    return new kritaView( this, parent, name );
}

bool kritaPart::loadXML( QIODevice *, const QDomDocument & )
{
    /// @todo load the document from the QDomDocument
    return true;
}

QDomDocument kritaPart::saveXML()
{
    /// @todo save the document into a QDomDocument
    return QDomDocument();
}


void kritaPart::paintContent( QPainter& painter, const QRect& rect, bool /*transparent*/,
                                double /*zoomX*/, double /*zoomY*/ )
{
    // ####### handle transparency

    // Need to draw only the document rectangle described in the parameter rect.
    int left = rect.left() / 20;
    int right = rect.right() / 20 + 1;
    int top = rect.top() / 20;
    int bottom = rect.bottom() / 20 + 1;

    for( int x = left; x < right; ++x )
        painter.drawLine( x * 20, top * 20, x * 20, bottom * 20 );
    for( int y = left; y < right; ++y )
        painter.drawLine( left * 20, y * 20, right * 20, y * 20 );
}

#include "krita_part.moc"
