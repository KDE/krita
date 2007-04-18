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

#ifndef TOKENSTYLEELEMENT_H
#define TOKENSTYLEELEMENT_H

#include "kformuladefs.h"
#include "RowElement.h"

KFORMULA_NAMESPACE_BEGIN

/**
 * This class handles mathematical style attributes common to token elements,
 * as explained in MathML Spec, Section 3.2.2.
 *
 * It is in charge of reading and saving elements' attributes and setting
 * rendering information according to these attributes.
 */
class TokenStyleElement : public RowElement {
    typedef RowElement inherited;
public:

    TokenStyleElement( BasicElement* parent = 0 );

protected:
    /**
     * Return RGB string from HTML Colors. See HTML Spec, section 6.5
     */
    QString getHtmlColor( const QString& colorStr );

};

KFORMULA_NAMESPACE_END

#endif // TOKENSTYLEELEMENT_H
