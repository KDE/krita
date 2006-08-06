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

#ifndef MATHMLLOADER_H
#define MATHMLLOADER_H

/**
 * @file
 */

#include <QDomDocument>
#include <QStack>

namespace KFormula {

class FormulaContainer;
class BasicElement;

/**
 * @short MathMLLoader is KFormula's MathML parser
 *
 * The MathMLLoader class is designed after the builder pattern. It parses the
 * contents of a MathML tree which is passed through the parse() method.
 * As result of the parsing the MathMLLoader builds the element structure in the
 * FormulaContainer passed during construction. The FormulaContainer can get formulas
 * from several sources which will then be tied together. If an error occurs during the
 * parsing it can be obtained with the parseError() method.
 *
 * @author Martin Pfeiffer <hubipete@gmx.net>
 * @since 2.0
 */
class MathMLLoader {
public:
    /**
     * The constructor
     * @param container The @see FormulaContainer to parse in
     */
    MathMLLoader( FormulaContainer* container );

    /**
     * Do the actual parsing
     * @return true when the parsing succeed without error otherwise false
     * @param mmldoc The QDomDocument which contains the MathML
     */
    bool parse( const QDomDocument& mmldoc );

    /// @return The error ocured during parsing the document
    void parseError();

protected:
    void math();

    // Token Elements
    void mi();
    void mn();
    void mo();
    void mtext();
    void mspace();
    void ms();
    void mglyph();

    // General Layout Schemata
    void mrow();
    void mfrac();
    void msqrt();
    void mroot();
    void mstyle();
    void merror();
    void mpadded();
    void mphantom();
    void mfenced();
    void menclose();
	    
    // Script and Limit Schemata
    void msub();
    void msup();
    void msubsup();
    void munder();
    void mover();
    void munderover();
    void mmultiscripts();

    // Tables and Matrices
    void mtable();
    void mlabeledtr();
    void mtr();
    void mtd();
    void maligngroup();
    void malignmark();

private:
    /// Decide which function to use to parse the m_currentElement
    void processElement();
    
    /// The @see FormulaContainer to contain the results of parsing 
    FormulaContainer* m_formulaContainer;

    /// True if the current document uses the OASIS namespace
    bool m_oasisNamespace;

    /// The currently parsed QDomElement
    QDomElement m_currentElement;

    /// The parent of m_currentElement
    QDomElement m_currentParentElement;

    /// A stack holding on top always the current BasicElement used as parent
    QStack<BasicElement*> m_parentElements;
};

} // namespace KFormula

#endif // MATHMLLOADER_H
