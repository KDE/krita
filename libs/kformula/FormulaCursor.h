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

#include <QDomElement>
#include "kformuladefs.h"

class QPainter;

namespace KFormula {

class BasicElement;


class SequenceElement;

/**
 * @short The cursor being moved through the formula
 *
 * Each element implements its own cursor behaviour. There are always at least two
 * positions the cursor can have in an element: before and after. Only in sequences
 * there are more positions possible. Before the element is 0, after it 1 and so on.
 */
class FormulaCursor {
public:
    /**
     * The constructor - set the FormulaCursor right to the beginning
     * @param element The element the FormulaCursor is set to at the beginning
     */
    FormulaCursor( BasicElement* element );

    /// @return The element the FormulaCursor is currently inside
    BasicElement* currentElement() const;

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
    void setCursorTo( BasicElement* current, int position );

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

    /**
     * Make the cursor move a whole element
     * @param wordMovement When true the cursor moves a whole element
     */
    void setWordMovement( bool wordMovement );
    
    

    
    

    // where the cursor and the mark are
    int getPos() const { return m_positionInElement; }
    int getMark() const { return markPos; }

    /**
     * Tells whether the cursor has changed since last cleaning.
     */
    bool hasChanged() const { return hasChangedFlag; }

    /**
     * Resets the cursor's change flag. The widget calls this
     * if it has drawn the cursor.
     */
    void clearChangedFlag() { hasChangedFlag = false; }

    /**
     * Sets the cursor in linear mode. This means you can visit every
     * element just by moving left and right.
     */
    void setLinearMovement(bool linear) { linearMovement = linear; }

    /**
     * Moves the cursor inside the element. Selection is turned off.
     */
    void goInsideElement(BasicElement* element);

    /**
     * Removes the current selected children and returns them.
     * The cursor needs to be normal (that is be inside a SequenceElement)
     * for this to have any effect.
     */
    void remove(QList<BasicElement*>&,
                Direction = beforeCursor);


    /**
     * Replaces the current selection with the supplied element.
     * The replaced elements become the new element's main child's content.
     */
    void replaceSelectionWith(BasicElement*,
                              Direction = beforeCursor);

    /**
     * Replaces the element the cursor points to with its main child's
     * content.
     */
    BasicElement* replaceByMainChildContent(Direction = beforeCursor);

    /**
     * Trys to find the element we are the main child of and replace
     * it with our content.
     *
     * This is simply another form of replaceByMainChildContent. You
     * use this one if the cursor is normalized and inside the main child.
     */
    BasicElement* removeEnclosingElement(Direction = beforeCursor);

    /**
     * Returns wether the element the cursor points to should be replaced.
     * Elements are senseless as soon as they only contain a main child.
     */
    bool elementIsSenseless();


    // The range that is selected. Makes no sense if there is
    // no selection.

    int getSelectionStart() const { return qMin(getPos(), getMark()); }
    int getSelectionEnd() const { return qMax(getPos(), getMark()); }


    /**
     * Sets the cursor to a new position.
     * This gets called from the element that wants
     * to own the cursor. It is a mistake to call this if you aren't
     * an element.
     *
     * If you provide a mark >= 0 the selection gets turned on.
     * If there is a selection and you don't provide a mark the
     * current mark won't change.
     */
    void setTo( BasicElement* element, int cursor, int mark=-1);

    void setPos(int pos);
    void setMark(int mark);


    /**
     * Moves the cursor to a normal position. That is somewhere
     * inside a SequenceElement.
     * You need to call this after each removal because the cursor
     * might point to some non existing place.
     */
    void normalize(Direction direction = beforeCursor);


    /**
     * Returns the sequence the cursor is in if we are normal. If not returns 0.
     */
    SequenceElement* normal();
    const SequenceElement* normal() const;

    /**
     * Selects the element the cursor points to (stands after)
     * if there is such an element and if there is no selection.
     */
    void selectActiveElement();

    /**
     * The element is going to leave the formula with and all its children.
     */
    void elementWillVanish(BasicElement* element);

    /**
     * @returns whether we are allowed to alter the document.
     */
    bool isReadOnly() const;

    /**
     * Puts the widget in read only mode.
     */
    void setReadOnly(bool ro) { readOnly = ro; }

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

   


    /**
     * Returns the child the cursor points to. Depending on the
     * direction this might be the child before or after the
     * cursor.
     *
     * Might be 0 is there is no such child.
     */
    BasicElement* getActiveChild(Direction direction);

    /**
     * Returns the child that is currently selected.
     *
     * Might be 0 is there is no such child. e.g. if there are more
     * than one element selected.
     */
    BasicElement* getSelectedChild();

    /**
     * Tells whether we currently point to the given elements
     * main child and to the place behind its last child.
     */
    bool pointsAfterMainChild(BasicElement*);

    /**
     * Sets the selection according to the shift key.
     */
    void handleSelectState(int flag);

    /**
     * The position of the mark. If we are in selection mode this
     * is the other side of the selected area.
     * Note that the mark always belongs to the same SequenceElement
     * as the cursor.
     */
    int markPos;

    /**
     * Tells whether there is a selection area.
     * (This is not equal to (markPos != -1).)
     */
    bool selectionFlag;

    /**
     * Tells whether we want to travel through all elements by
     * left and right movement.
     */
    bool linearMovement;

    /**
     * The point in the middle of the cursor. Gets updated
     * each time the cursor is drawn.
     */
    LuPixelPoint cursorPoint;

    /**
     * The area that is covered by the cursor. Gets updated
     * each time the cursor is drawn.
     */
    LuPixelRect cursorSize;

    /**
     * Tells whether the cursor has been changed. This is set
     * by any of the setSomething methods. It's used by the
     * widget the cursor belongs to.
     */
    bool hasChangedFlag;

    /**
     * Whether we are only allowed to read.
     */
    bool readOnly;
};

} // namespace KFormula

#endif // FORMULACURSOR_H
