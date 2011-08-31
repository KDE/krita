/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
                      Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>
                 2006 Martin Pfeiffer <hubipete@gmx.net>
                 2009 Jeremias Epperlein <jeeree@web.de>

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
   Boston, MA 02110-1301, USA.
*/

#ifndef TABLEDATAELEMENT_H
#define TABLEDATAELEMENT_H

#include "RowElement.h"
#include "kformula_export.h"

/**
 * @short Implementation of the MathML mtd element
 * 
 * The lines behaviour is (a little) different from that
 * of ordinary sequences. Its MathML tag is \<mtd\>.
 */
class KOFORMULA_EXPORT TableDataElement : public RowElement {
public:
    /// The standard constructor
    TableDataElement( BasicElement* parent = 0 );

//    /**
//     * Calculate the size of the element and the positions of its children
//     * @param am The AttributeManager providing information about attributes values
//     */
//    void layout( const AttributeManager* am );
    
    /// @return The element's ElementType
    ElementType elementType() const;
    
    virtual bool moveCursor ( FormulaCursor& newcursor, FormulaCursor& oldcursor );
    
    /// @return The default value of the attribute for this element
    QString attributesDefaultValue( const QString& attribute ) const; 
};

#endif // TABLEDATAELEMENT_H
