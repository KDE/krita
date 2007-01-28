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

#ifndef FORMULACURSOR_H
#define FORMULACURSOR_H

#include <QString>
#include <QStack>

class QPainter;

namespace FormulaShape {

class BasicElement;

/**
 * @short The cursor being moved through a formula
 *
 * The FormulaTool instanciates FormulaCursor to move around in the formula and edit
 * it. Each element can implement special cursor behaviour for its children. There
 * are always at least two positions the cursor can have in an element: before and
 * after the element. Only in mrow and some token elements there are more positions
 * possible and in a BasicElement there is only one position. Before the element is
 * position 0, after it position 1 and so on.
 * FormulaTool calls the moveLeft, moveRight, moveUp and moveDown methods. It also
 * sets with setSelection and setWordMovement the further behaviour of the cursor
 * according to the modifiers the user pressed.
 * The FormulaCursor class is also used to save a certain place in the formula. With
 * the currentElement() and position() methods it is possible to act with a special
 * place.
 */
class FormulaCursor {
public:
    /**
     * The constructor - set the FormulaCursor right to the beginning
     * @param element The element the FormulaCursor is set to at the beginning
     */
    explicit FormulaCursor( BasicElement* element );

    /**
     * Draw the cursor to the given QPainter
     * @param painter The QPainter the cursor draws itsself to
     */
    void paint( QPainter &painter ) const;

    /**
     * Set the cursor to a new position
     * @param current The new element the pointer is inside
     * @þaram position The position in the new element the cursor is set to
     */
    void moveCursorTo( BasicElement* current, int position );

    /// Move the cursor to the left
    void moveLeft();

    /// Move the cursor to the right
    void moveRight();

    /// Move the cursor up
    void moveUp();

    /// Move the cursor down
    void moveDown();

    /// Move the cursor to the first position in the current element
    void moveHome();

    /// Move the cursor to the last position in the current element
    void moveEnd();

    /// @return whether the cursor is at the first position
    bool isHome() const;

    /// @return whether the cursor is at the last position
    bool isEnd() const;

    /**
     * Make the cursor selecting
     * @param selecting When true the cursor is selecting
     */ 
    void setSelecting( bool selecting );

    /// @return @c true when the cursor is selecting
    bool hasSelection() const;

    /// @return The element the FormulaCursor is currently inside
    BasicElement* currentElement() const;

    /// @return The current position in m_currentElement
    int position() const;

    /**
     * Make the cursor move a whole element
     * @param wordMovement When true the cursor moves a whole element
     */
    void setWordMovement( bool wordMovement );

private:
    /// The element that currently owns the cursor
    BasicElement* m_currentElement;
    
    /// The cursor's position inside m_ownerElement - 0 based index
    int m_positionInElement;

    /// Indicates whether the cursor should move a whole element
    bool m_wordMovement;

    /// Indicates whether the cursor is currently selecting
    bool m_selecting;
    
    QStack<BasicElement*> m_selectedElements;
};

} // namespace FormulaShape

#endif // FORMULACURSOR_H
