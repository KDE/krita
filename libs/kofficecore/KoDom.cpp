/* This file is part of the KDE project
   Copyright (C) 2004 David Faure <faure@kde.org>

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

#include "KoDom.h"

QDomElement KoDom::namedItemNS( const QDomNode& node, const char* nsURI, const char* localName )
{
    QDomNode n = node.firstChild();
    for ( ; !n.isNull(); n = n.nextSibling() ) {
        if ( n.isElement() && n.localName() == localName && n.namespaceURI() == nsURI )
            return n.toElement();
    }
    return QDomElement();
}
