/* This file is part of the KDE project
   Copyright (C)  2006 Martin Pfeiffer <hubipete@gmx.net>

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

#ifndef FORMULASHAPE_H
#define FORMULASHAPE_H

#include <KoShape.h>

#include <QDomDocument>

class KoXmlWriter;

namespace KFormula {

class BasicElement;

/**
 * @short The flake shape for a formula
 * @author Martin Pfeiffer <hubipete@gmx.net>
 * @since 2.0
 */
class KOFORMULA_EXPORT FormulaShape : public KoShape {
public:
    /// The basic constructor
    FormulaShape();

    /// The basic destructor
    ~FormulaShape();

    /// inherited from KoShape
    void paint( QPainter &painter, KoViewConverter &converter );

    /// @return The BasicElement at the highest level in the formula tree
    BasicElement* formulaElement() const;

    /**
     * Save the formula as MathML
     * @param writer 
     * @param oasisFormat If true the MathMl is saved to OASIS conform MathML
    */
    void saveMathML( KoXmlWriter* writer, bool oasisFormat = false );

    /**
     * Load the formula from the specified file containing MathML
     * @param doc The DomDocument to load from
     * @param oasisFormat If true the formula is read from OASIS conform MathML
     */
    void loadMathML( const QDomDocument &doc, bool oasisFormat = false );

private:
    /// The element at the highest level in the formula tree
    BasicElement* m_formulaElement;
};

} // namespace KFormula

#endif // FORMULASHAPE_H
