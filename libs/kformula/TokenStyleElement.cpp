/* This file is part of the KDE project
   Copyright (C) 2006-2007 Alfredo Beaumont Sainz <alfredo.beaumont@gmail.com>

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

#include <qpainter.h>

#include <kdebug.h>

#include "BasicElement.h"
#include "TokenStyleElement.h"

KFORMULA_NAMESPACE_BEGIN

TokenStyleElement::TokenStyleElement( BasicElement* parent ) : RowElement( parent )
{
}


/**
 * Return RGB string from HTML Colors. See HTML Spec, section 6.5
 */
QString TokenStyleElement::getHtmlColor( const QString& colorStr ){

    QString colorname = colorStr.toLower();

    if ( colorname ==  "black" ) 
        return "#000000";
    if ( colorname == "silver" )
        return "#C0C0C0";
    if ( colorname == "gray" )
        return "#808080";
    if ( colorname == "white" )
        return "#FFFFFF";
    if ( colorname == "maroon" )
        return "#800000";
    if ( colorname == "red" )
        return "#FF0000";
    if ( colorname == "purple" )
        return "#800080";
    if ( colorname == "fuchsia" )
        return "#FF00FF";
    if ( colorname == "green" )
        return "#008000";
    if ( colorname == "lime" )
        return "#00FF00";
    if ( colorname == "olive" )
        return "#808000";
    if ( colorname == "yellow" )
        return "#FFFF00";
    if ( colorname == "navy" )
        return "#000080";
    if ( colorname == "blue")
        return "#0000FF";
    if ( colorname == "teal" )
        return "#008080";
    if ( colorname == "aqua" )
        return "#00FFFF";
    kWarning( DEBUGID ) << "Invalid HTML color: " << colorname << endl;
    return "#FFFFFF"; // ### Arbitrary selection of default color
}


KFORMULA_NAMESPACE_END
