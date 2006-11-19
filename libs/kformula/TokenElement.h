/* This file is part of the KDE project
   Copyright (C) 2006 Alfredo Beaumont Sainz <alfredo.beaumont@gmail.com>

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

#ifndef TOKENELEMENT_H
#define TOKENELEMENT_H

#include "formulacursor.h"
#include "tokenstyleelement.h"
#include "sequenceelement.h"
#include "contextstyle.h"

KFORMULA_NAMESPACE_BEGIN

class TokenElement : public TokenStyleElement {
    typedef TokenStyleElement inherited;
public:
    TokenElement( BasicElement* parent = 0 );

	virtual int buildChildrenFromMathMLDom(QPtrList<BasicElement>& list, QDomNode n);

    virtual QString getElementName() const { return "mtext"; }
protected:
    QString getCharFromEntity( const QString& entity );

    /**
     * @returns true if the sequence contains only text.
     */
    virtual bool isTextOnly() const { return m_textOnly; }

    /**
     * Space around sequence
     */
    virtual luPt getSpaceBefore( const ContextStyle& context, 
                                 ContextStyle::TextStyle tstyle,
                                 double factor );
    virtual luPt getSpaceAfter( const ContextStyle& context, 
                                 ContextStyle::TextStyle tstyle,
                                 double factor );
private:
    bool m_textOnly;
};

KFORMULA_NAMESPACE_END

#endif // TOKENELEMENT_H
