/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
	              Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>

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

#ifndef FORMULACURSOR_H
#define FORMULACURSOR_H

#include <QString>
#include <QList>

#include "BasicElement.h"
#include "kformuladefs.h"

KFORMULA_NAMESPACE_BEGIN

class FormulaElement;
class MatrixElement;
class NameSequence;
class RootElement;
class SymbolElement;
class TextElement;


/**
 * The selection. This might be a position selection or
 * an area. Each view will need one FormulaCursor.
 *
 * The @ref Container always uses the cursor to operate on
 * the element tree.
 *
 * Note that it is up to the elements to actually move the cursor.
 * (The cursor has no chance to know how.)
 */
class FormulaCursor {

    // Yes, we do have a friend.
    friend class SequenceElement;

public:

    /**
     * Creates a cursor and puts is at the beginning
     * of the formula.
     *
     * @param element the formula the cursor point to. This must not be 0.
     */
    FormulaCursor(FormulaElement* element);

    FormulaCursor& operator= (const FormulaCursor&);

    // where the cursor and the mark are
    int getPos() const { return cursorPos; }
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
     * Returns wether we are in selection mode.
     */
    bool isSelectionMode() const { return selectionFlag; }

    /**
     * Returns wether there actually is a selection.
     */
    bool isSelection() const { return selectionFlag && (getPos() != getMark()); }

    /**
     * Sets the selection mode.
     */
    void setSelection(bool selection) { selectionFlag = selection; hasChangedFlag = true; }

    /**
     * Calculates the size of the cursor. Needs to be called before
     * the cursor can be drawn.
     */
    void calcCursorSize( const ContextStyle& context, bool smallCursor );

    /**
     * Draws the cursor at its current position.
     * The cursor will always be drawn in xor mode.
     */
    void draw( QPainter&, const ContextStyle& context, bool smallCursor, bool activeCursor );


    // simple cursor movement.

    void moveLeft(int flag = NormalMovement);
    void moveRight(int flag = NormalMovement);
    void moveUp(int flag = NormalMovement);
    void moveDown(int flag = NormalMovement);

    void moveHome(int flag = NormalMovement);
    void moveEnd(int flag = NormalMovement);

    /** @returns whether the cursor is at the first position. */
    bool isHome() const;

    /** @returns whether the cursor is at the last position. */
    bool isEnd() const;

    // how to travel

    bool getLinearMovement() const { return linearMovement; }

    /**
     * Sets the cursor in linear mode. This means you can visit every
     * element just by moving left and right.
     */
    void setLinearMovement(bool linear) { linearMovement = linear; }

    /**
     * Moves the cursor inside the element. Selection is turned off.
     */
    void goInsideElement(BasicElement* element);

    // mouse selection

    void mousePress( const LuPixelPoint&, int flags );
    void mouseMove( const LuPixelPoint&, int flags );
    void mouseRelease( const LuPixelPoint&, int flags );

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
    void setTo(BasicElement* element, int cursor, int mark=-1);

    void setPos(int pos);
    void setMark(int mark);


    /**
     * The element we are in. In most cases this is a SequenceElement.
     * There is no way to place a cursor outside a SequenceElement by
     * normal movement.
     * But in special cases (e.g. if you remove an index from an
     * IndexElement) the cursor can be placed to odd places. This is
     * the reason why you have to normalize the cursor after each
     * removal.
     */
    BasicElement* getElement() { return current; }
    const BasicElement* getElement() const { return current; }


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
     * Stores the currently selected elements inside a dom.
     */
    void copy( QDomDocument& doc );

    /**
     * Inserts the elements that could be read from the dom into
     * the list. Returns true on success.
     */
    bool buildElementsFromDom( QDomElement root, QList<BasicElement*>& list );

    // undo/redo support

    /**
     * A black box that is supposed to contain everything
     * which is needed to describe a cursor. Only the cursor
     * itself is allowed to read it.
     */
    class CursorData {
        friend class FormulaCursor;
        BasicElement* current;
        int cursorPos;
        int markPos;
        bool selectionFlag;
        bool linearMovement;
        bool readOnly;

        CursorData(BasicElement* c,
                   int pos, int mark, bool selection, bool linear, bool ro)
            : current(c), cursorPos(pos), markPos(mark),
              selectionFlag(selection), linearMovement(linear),
              readOnly(ro) {}
    };

    /**
     * Creates a new CursorData object that describes the cursor.
     * It's up to the caller to delete this object.
     */
    CursorData* getCursorData();

    /**
     * Sets the cursor to where the CursorData points to. No checking is done
     * so you better make sure the point exists.
     */
    void setCursorData(CursorData* data);

    /**
     * The element is going to leave the formula with and all its children.
     */
    void elementWillVanish(BasicElement* element);

    /**
     * A new formula has been loaded. Our current element has to change.
     */
    void formulaLoaded(FormulaElement* rootElement);

    /**
     * @returns the point inside the formula widget where the cursor is.
     */
    const LuPixelPoint& getCursorPoint() const { return cursorPoint; }

    /**
     * @returns the area the cursor is currently on.
     */
    const LuPixelRect& getCursorSize() const { return cursorSize; }
    void addCursorSize( const LuPixelRect& rect ) { cursorSize |= rect; }

    /**
     * @returns whether we are allowed to alter the document.
     */
    bool isReadOnly() const;

    /**
     * Puts the widget in read only mode.
     */
    void setReadOnly(bool ro) { readOnly = ro; }

private:

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
     * The element the cursor is inside right now.
     */
    BasicElement* current;

    /**
     * The position the cursor in on inside the element.
     * Might be anything from 0 to current->children->size().
     *
     * This is where new elements are put in.
     */
    int cursorPos;

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

KFORMULA_NAMESPACE_END

#endif // FORMULACURSOR_H
