/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
                      Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>
                 2006-2007 Martin Pfeiffer <hubipete@gmx.net>

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

#ifndef FORMULAELEMENT_H
#define FORMULAELEMENT_H

#include "RowElement.h"
#include "kformula_export.h"
#include <QObject>

/**
 * @short The element of a formula at the highest position.
 *
 * A formula consists of a tree of elements. The FormulaElement is the root of this
 * tree and therefore is the only element that doesn't have a parent element.
 * It's functionality is reduced to layouting its children in a different way. It is
 * the element with highest size and can also dictate the size to all other elements. 
 */
class KOFORMULA_EXPORT FormulaElement : public RowElement {
public:
    /// The standard constructor
    FormulaElement();

    /// @return The element's ElementType
    virtual ElementType elementType() const;

    virtual void writeMathMLAttributes(KoXmlWriter* writer) const;
};

#endif // FORMULAELEMENT_H
