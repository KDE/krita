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

#ifndef KFORMULASHAPE_H
#define KFORMULASHAPE_H

#include <KoShape.h>

namespace KFormula {

class FormulaRenderer;
class FormulaContainer;

/**
 * @short The flake shape for a formula
 */
class KOFORMULA_EXPORT FormulaShape : public KoShape {
public:
    /// The basic constructor
    FormulaShape();

    /// The basic destructor
    ~FormulaShape();

    /// inherited from KoShape
    virtual void paint( QPainter &painter, KoViewConverter &converter ) = 0;

    /**
     * Save the formula as MathML
     * @param stream
     * @param oasisFormat If true the MathMl is saved to OASIS conform MathML
    */
    void saveMathML( QTextStream& stream, bool oasisFormat = false );

    /**
     * Load the formula from the specified file containing MathML
     * @param doc The DomDocument to load from
     * @param oasisFormat If true the formula is read from OASIS conform MathML
     * @return true if success
     */
    bool loadMathML( const QDomDocument &doc, bool oasisFormat = false );

protected:
    void mousePressEvent( QMouseEvent* event );
    void mouseReleaseEvent( QMouseEvent* event );
    void mouseDoubleClickEvent( QMouseEvent* event );
    void mouseMoveEvent( QMouseEvent* event );

    void keyPressEvent( QKeyEvent* event );
    void focusInEvent( QFocusEvent* event );
    void focusOutEvent( QFocusEvent* event );


private:
    /// The formula data
    FormulaContainer* m_formulaContainer;

    /// The @see FormulaCursor used to alter the formula data in @ref m_formulaContainer
    FormulaCursor* m_formulaCursor;

    /// The formula renderer used to paint the formula
    FormulaRenderer* m_formulaRenderer;
};

} // namespace KFormula

#endif // KFORMULASHAPE_H
