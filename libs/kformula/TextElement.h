/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
	              Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>
   Copyright (C) 2006 Martin Pfeiffer <hubipete@gmx.net>
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
   Boston, MA 02110-1301, USA.
*/

#ifndef TEXTELEMENT_H
#define TEXTELEMENT_H

#include <QFont>
#include <QString>

#include "BasicElement.h"


namespace FormulaShape {

class SymbolTable;
 
/**
 * @short Implementation of the MathML
 * An element that represents one char.
 */
class TextElement : public BasicElement {
public:
    /// The standard constructor
    explicit TextElement(QChar ch = ' ', bool beSymbol = false, BasicElement* parent = 0);

    /**
     * Obtain a list of all child elements of this element
     * @return a QList with pointers to all child elements
     */
    const QList<BasicElement*> childElements();

    void writeMathML( KoXmlWriter* writer, bool oasisFormat = false );

    

    /**
     * @returns true if we don't want to see the element.
     */
    virtual bool isInvisible() const;

    /**
     * Render the element to the given QPainter
     * @param painter The QPainter to paint the element to
     */
    virtual void paint( QPainter& painter, const AttributeManager* am );

    /**
     * Calculate the size of the element and the positions of its children
     * @param am The AttributeManager providing information about attributes values
     */
    virtual void layout( const AttributeManager* am );
    
protected:
    //Save/load support

    virtual void writeMathMLContent( KoXmlWriter* , bool ) const ;

    /**
     * @returns the char that is used to draw with the given font.
     */
    QChar character() const { return m_character; }

    /**
     * @returns the font to be used for the element.
     */
	QFont getFont( const AttributeManager* am );

    const SymbolTable& getSymbolTable() const;

private:

    /**
     * Our content.
     */
    QChar m_character;

    /**
     * Whether this character is a symbol.
     */
    bool symbol;

};

} // namespace FormulaShape

#endif // TEXTELEMENT_H
