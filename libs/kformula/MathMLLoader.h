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

#include <QDomDocument>

namespace KFormula {

class FormulaContainer;

/**
 * @short MathMLLoader is KFormula's MathML parser
 *
 * The MathMLLoader class is designed after the builder pattern. It parses the
 * contents of a MathML tree which is passed through the @ref parse() method.
 * As result of the parsing the MathMLLoader builds the element structure in the
 * FormulaContainer passed during construction. The FormulaContainer can get formulas
 * from several sources which will then be tied together. If an error occurs during the
 * parsing it can be obtained with the parseError() method.
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
     * @return True when the parsing succeed without error otherwise false
     * @param mmldoc The QDomDocument which contains the MathML
     * @param oasisFormat Indicates if the MathML is inside the OASIS namespace
     */
    bool parse( const QDomDocument& mmldoc, bool oasisFormat = false );

    /// @return The error ocured during parsing the document
    void parseError();

private:
    /// The @see FormulaContainer to contain the results of parsing 
    FormulaContainer* m_formulaContainer;
};

} // namespace KFormula

#endif // MATHMLLOADER_H
