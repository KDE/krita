/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
	              Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>
		 2006 Martin Pfeiffer <hubipete@gmx.net>

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

#ifndef MATRIXENTRYELEMENT_H
#define MATRIXENTRYELEMENT_H

#include "SequenceElement.h"

namespace KFormula {
	
/**
 * @short The class representing an entry in a matrix
 * 
 * The lines behaviour is (a little) different from that
 * of ordinary sequences. Its MathML tag is \<mtd\>.
 *
 * @since 2.0
 */
class MatrixEntryElement : public SequenceElement {
public:
    /// The standard constructor
    MatrixEntryElement( BasicElement* parent = 0 );

    /// @return a list of all children of this class                           
    const QList<BasicElement*>& childElements();

    void readMathML( const QDomElement& element );
    
    void writeMathML( const KoXmlWriter* writer, bool oasisFormat = false );






    /// Calculates our width and height and our children's parentPosition.
    virtual void calcSizes( const ContextStyle& context, ContextStyle::TextStyle tstyle,
                                      ContextStyle::IndexStyle istyle );
	
    virtual void registerTab( BasicElement* tab );
	
        /**
 	 * This is called by the container to get a command depending o                                                                                                         * the current cursor position (this is how the element gets chosen)
         * and the request.
	 * @returns the command that performs the requested action with the
	 * containers active cursor.
	 */
/*	 virtual KCommand* buildCommand( Container*, Request* );
	 virtual KCommand* input( Container* container, QKeyEvent* event );
	 virtual KCommand* input( Container* container, QChar ch );*/
	 int tabCount() const { return tabs.count(); }
	 BasicElement* tab( int i ) { return tabs.at( i ); }
		 
         /// Change the width of tab i and move all elements after it.
         void moveTabTo( int i, luPixel pos );

	 /// Return the greatest tab number less than pos.
	 int tabBefore( int pos );
	 
         /// Return the position of tab i.
         int tabPos( int i );


protected:
    /// Draws the element internally, means it paints into m_elementPath
    void drawInternal();

    void readMathMLAttributes( const QDomElement& element );


private:
    QList<BasicElement*> tabs;
};

} // namespace KFormula

#endif
