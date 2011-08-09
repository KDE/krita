/* This file is part of the KDE project
   Copyright (C) 2009 Jeremias Epperlein <jeeree@web.de>

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

#ifndef FIXEDELEMENT_H
#define FIXEDELEMENT_H

#include "BasicElement.h"
#include "kformula_export.h"
#include <KoXmlReader.h>


class FormulaCursor;
class QPainterPath;
class RowElement;
/**
 * @short Abstract Base Class for MathML elements with fixed number of children
 *
 */

class KOFORMULA_EXPORT FixedElement : public BasicElement {
public:
    /// The standard constructor
    FixedElement( BasicElement* parent = 0 );

    /// The standard destructor
    virtual ~FixedElement();

    virtual bool acceptCursor ( const FormulaCursor& cursor );

    BasicElement* elementBefore(int position) const;

    BasicElement* elementAfter(int position) const;

    virtual BasicElement* elementNext(int position) const;

    virtual QLineF cursorLine ( int position ) const;

    bool loadElement(KoXmlElement& tmp, RowElement** child);

    virtual QPainterPath selectionRegion ( const int pos1, const int pos2 ) const;

    /// inherited from BasicElement
    virtual int positionOfChild(BasicElement* child) const;

    virtual int endPosition() const;

protected:
    bool moveHorSituation(FormulaCursor& newcursor, FormulaCursor& oldcursor,int pos1, int pos2);
    bool moveVertSituation(FormulaCursor& newcursor, FormulaCursor& oldcursor,int pos1, int pos2);
    bool moveSingleSituation(FormulaCursor& newcursor, FormulaCursor& oldcursor,int pos);
};

#endif // ROWELEMENT_H

