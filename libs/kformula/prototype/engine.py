"""This file is part of the KDE project
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
   Boston, MA 02110-1301, USA.
"""

from qt import *


class BasicElement:
    """The interface for every element."""
    
    def __init__(self, parent):
        self.parent = parent
        self.size = QSize()
        self.pos = QPoint()


    def x(self): return self.pos.x()
    def y(self): return self.pos.y()
    def setX(self, x): self.pos.setX(x)
    def setY(self, y): self.pos.setY(y)

    def width(self): return self.size.width()
    def height(self): return self.size.height()
    def setWidth(self, w): self.size.setWidth(w)
    def setHeight(self, h): self.size.setHeight(h)

    def globalPos(self):
        """Returns the pos in global Coords."""
        x = y = 0
        element = self
        while element != None:
            x += element.x()
            y += element.y()
            element = element.parent
        return QPoint(x, y)

    def elementAt(self, point, startPoint):
        """Returns the element that is at position point.
        `None' if there is no element there."""
        x = point.x() - startPoint.x()
        if x >= 0 and x < self.width():
            y = point.y() - startPoint.y()
            if y >= 0 and y < self.height():
                return self
    
    def moveLeft(self, cursor, fromElement):
        """Enters this element while moving to the left from
        the element `fromElement'. Searched for cursor position inside
        this element of left of it."""
        pass
    
    def moveRight(self, cursor, fromElement):
        """Enters this element while moving to the right from
        the element `fromElement'. Searched for cursor position inside
        this element of right of it."""
        pass

    def moveUp(self, cursor, fromElement):
        pass

    def moveDown(self, cursor, fromElement):
        pass
    
    def formula(self):
        """Returns the FormulaElement we are a child of."""
        return self.parent.formula()

    def draw(self, painter, styleContext, startPoint):
        """Draws the whole thing. Including its children."""
        pass

    def calcSizes(self, styleContext):
        """Recalculates the size.
        position (relative our to parent), width and height
        will be stored in self.size,
        the midline offset in self.midline.

        Please note: It's up to a parent to store its children's position."""
        pass

    def mainChild(self):
        """Returns the most important child. `None' if there is None
        child at all."""
        return None

    def setMainChild(self, sequenceElement):
        """Defines the main child."""
        pass

    def makeSequence(self):
        """Packs the element into a new SequenceElement."""
        return SequenceElement(self)

    def removeChild(self, cursor, element):
        """Removes the given child. If this happens to be the main
        child we remove ourself, too.
        The cursor has to be inside the child which is going to be
        removed."""
        pass


class SequenceElement (BasicElement):
    """The element that contains a number of children.
    The children are aligned in one line."""
    
    def __init__(self, parent):
        BasicElement.__init__(self, parent)
        self.children = []

    def elementAtCursor(self, cursor):
        """Returns the element before the cursor."""
        if cursor.pos() > 0:
            return self.children[cursor.pos()-1]
        
    def elementAt(self, point, startPoint):
        r = BasicElement.elementAt(self, point, startPoint)
        if r != None:
            for child in self.children:
                r = child.elementAt(point, QPoint(startPoint.x()+child.x(),
                                                  startPoint.y()+child.y()))
                if r != None:
                    return r
            return self

        
    def moveLeft(self, cursor, fromElement):

        # Our parent asks us for a cursor position. Found.
        if fromElement == self.parent:
            cursor.set(self, len(self.children))

        # We already owned the cursor. Ask next child then.
        elif fromElement == self:
            if cursor.pos() > 0:
                if cursor.isSelection():
                    cursor.set (self, cursor.pos()-1)
                else:
                    self.children[cursor.pos()-1].moveLeft(cursor, self)
            else:
                # Needed because FormulaElement derives this.
                if self.parent != None:
                    self.parent.moveLeft(cursor, self)

        # The cursor came from one of our children or
        # something is wrong.
        else:
            fromPos = self.children.index(fromElement)
            cursor.set(self, fromPos)
            if cursor.isSelection():
                if not cursor.mouseMark():
                    cursor.setMarkPos(fromPos+1)

    
    def moveRight(self, cursor, fromElement):

        # Our parent asks us for a cursor position. Found.
        if fromElement == self.parent:
            cursor.set(self, 0)

        # We already owned the cursor. Ask next child then.
        elif fromElement == self:
            if cursor.pos() < len(self.children):
                if cursor.isSelection():
                    cursor.set (self, cursor.pos()+1)
                else:
                    self.children[cursor.pos()].moveRight(cursor, self)
            else:
                # Needed because FormulaElement derives this.
                if self.parent != None:
                    self.parent.moveRight(cursor, self)

        # The cursor came from one of our children or
        # something is wrong.
        else:
            fromPos = self.children.index(fromElement)
            cursor.set(self, fromPos+1)
            if cursor.isSelection():
                if not cursor.mouseMark():
                    cursor.setMarkPos(fromPos)
                    

    def moveUp(self, cursor, fromElement):
        if fromElement == self.parent:
            self.moveRight(cursor, self)
        else:
            if self.parent != None:
                self.parent.moveUp(cursor, self)


    def moveDown(self, cursor, fromElement):
        if fromElement == self.parent:
            self.moveRight(cursor, self)
        else:
            if self.parent != None:
                self.parent.moveDown(cursor, self)
            

    def moveHome(self, cursor):
        if cursor.isSelection():
            element = cursor.element()
            if element != self:
                while element.parent != self:
                    element = element.parent
                cursor.setMarkPos(self.children.index(element)+1)
        cursor.set(self, 0)

    def moveEnd(self, cursor):
        if cursor.isSelection():
            element = cursor.element()
            if element != self:
                while element.parent != self:
                    element = element.parent
                cursor.setMarkPos(self.children.index(element))
        cursor.set(self, len(self.children))

        
    def draw(self, painter, styleContext, startPoint):
        x, y = startPoint.x(), startPoint.y()
        if len(self.children) > 0:
            for child in self.children:
                cX = child.x()
                cY = child.y()
                child.draw(painter, styleContext, QPoint(x+cX, y+cY))

            # Debug
            #painter.setPen(Qt.green)
            #painter.drawRect(x, y, self.width(), self.height())
        else:
            painter.setPen(Qt.blue)
            painter.drawRect(x, y, self.width(), self.height())

    def calcSizes(self, styleContext):
        if len(self.children) > 0:
            x = self.x()
            y = self.y()
            width = toMidline = fromMidline = 0
            for child in self.children:
                child.calcSizes(styleContext)
                child.setX(width)
                width += child.width()
                if child.midline > toMidline:
                    toMidline = child.midline
                if child.height()-child.midline > fromMidline:
                    fromMidline = child.height() - child.midline

            self.setWidth(width)
            self.setHeight(toMidline+fromMidline)
            self.midline = toMidline

            for child in self.children:
                child.setY(self.midline - child.midline)

        else:
            self.setWidth(10)
            self.setHeight(10)
            self.midline = 5

    def mainChild(self):
        if len(self.children) > 0:
            return self.children[0]
        return None

    def setMainChild(self, sequenceElement):
        if len(self.children) > 0:
            self.children[0] = sequenceElement
            sequenceElement.parent = self
        else:
            self.addChild(sequenceElement)

    def makeSequence(self):
        return self


    def replaceCurrentSelection(self, cursor, element):
        """Replaces the currently selected sequence (the child before
        the cursor) with the given element. The replaced sequence
        becomes the main child of the new element."""

        # it is essential to set up the parent pointer for
        # the notification to work.
        element.parent = self
        
        seq = element.makeSequence()
        if cursor.isSelection():
            f = min(cursor.pos(), cursor.markPos())
            t = max(cursor.pos(), cursor.markPos())
            for i in range(f, t):
                child = self.children.pop(f)
                self.formula().elementRemoved(child)
                seq.addChild(child)
            self.children.insert(f, element)
            cursor.setMarkPos(-1)
            cursor.set(self, f+1)
        elif cursor.pos() > 0:
            seq.addChild(self.children[cursor.pos()-1])
            self.replaceChild(cursor, element)
        else:
            self.insertChild(cursor, element)
            
        element.setMainChild(seq)


    def replaceElementByMainChild(self, cursor, element):
        """Replaces the given element with the content of its main child.
        (The main child is always a SequenceElement.)"""
        assert element.parent == self
        self.formula().elementRemoved(element)
        
        seq = element.mainChild()
        pos = self.children.index(element)
        self.children.remove(element)
        for child in seq.children:
            self.children.insert(pos, child)
            child.parent = self
            pos += 1
        cursor.set(self, pos)
        self.formula().changed()


    def addChild(self, element):
        self.children.append(element)
        element.parent = self
        self.formula().changed()

    def insertChild(self, cursor, element):
        """Inserts the new element at the cursor position.
        The cursor is placed behind the new element."""
        pos = cursor.pos()
        self.children.insert(pos, element)
        element.parent = self
        cursor.set(self, pos+1)
        self.formula().changed()
        
    def replaceChild(self, cursor, element):
        """Replaces the element before the cursor with the new one.
        No range checking. Be careful."""
        self.children[cursor.pos()-1] = element
        element.parent = self
        self.formula().changed()

    def removeChild(self, cursor, element):
        self.formula().elementRemoved(element)
        cursor.set(self, self.children.index(element))
        self.children.remove(element)
        if len(self.children) == 0:
            if self.parent != None:
                self.parent.removeChild(cursor, self)
                return
        self.formula().changed()
        
    def removeChildAt(self, cursor):
        pos = cursor.pos()
        if cursor.isSelection():
            f = min(cursor.pos(), cursor.markPos())
            t = max(cursor.pos(), cursor.markPos())
            for i in range(f, t):
                child = self.children.pop(f)
                self.formula().elementRemoved(child)
            cursor.setMarkPos(-1)
            cursor.set(self, f)
            self.formula().changed()
        elif pos < len(self.children):
            self.children.pop(pos)
            self.formula().changed()
        else:
            if len(self.children) == 0:
                if self.parent != None:
                    self.parent.removeChild(cursor, self)
            
    def removeChildBefore(self, cursor):
        pos = cursor.pos()-1
        if cursor.isSelection():
            f = min(cursor.pos(), cursor.markPos())
            t = max(cursor.pos(), cursor.markPos())
            for i in range(f, t):
                child = self.children.pop(f)
                self.formula().elementRemoved(child)
            cursor.setMarkPos(-1)
            cursor.set(self, f)
            self.formula().changed()
        elif pos >= 0:
            self.children.pop(pos)
            cursor.set(self, pos)
            self.formula().changed()
        else:
            if len(self.children) == 0:
                if self.parent != None:
                    self.parent.removeChild(cursor, self)
        
    
    def globalCursorPos(self, pos):
        """Returns the position after the child at the position
        in global Coords."""
        point = self.globalPos()
        if pos < len(self.children):
            d = self.children[pos].x()
        else:
            if len(self.children) > 0:
                d = self.width()
            else:
                d = 2
                
        point.setX(point.x()+d)
        return point

    def countChildren(self):
        return len(self.children)
            

class FormulaElement (SequenceElement):
    """The main element.
    A formula consists of a FormulaElement and its children.
    The only element that has no parent."""

    def __init__(self, document):
        SequenceElement.__init__(self, None)
        self.document = document

    def formula(self):
        return self

    def changed(self):
        """Is called by its children if the formula changed in any way."""
        self.document.changed()

    def elementRemoved(self, element):
        """Gets called just before the element is removed from the
        tree. We need this to ensure that no cursor is left in the
        leaf that gets cut off.

        Caution! The object tree must still contain the element by the time
        you call this methode."""
        self.document.elementRemoved(element)
        

class TextElement (BasicElement):
    """One char."""
    
    def __init__(self, parent, char):
        BasicElement.__init__(self, parent)
        self.char = char

    def moveLeft(self, cursor, fromElement):
        self.parent.moveLeft(cursor, self)
        
    def moveRight(self, cursor, fromElement):
        self.parent.moveRight(cursor, self)

    def draw(self, painter, styleContext, startPoint):
        styleContext.setupPainter(painter)
        painter.drawText(startPoint.x(), startPoint.y()+self.baseline, self.char)
        #painter.drawRect(startPoint.x(), startPoint.y(), self.width(), self.height())

    def calcSizes(self, styleContext):
        fm = styleContext.fontMetrics()
        self.setWidth(fm.width(self.char))
        self.setHeight(fm.height())
        self.midline = self.height() / 2
        self.baseline = fm.ascent()


class IndexElement (BasicElement):
    """Up to four indexes in the four corners."""

    def __init__(self, contentElement):
        if contentElement != None:
            BasicElement.__init__(self, contentElement.parent)
            contentElement.parent = self
        else:
            BasicElement.__init__(self, None)
            
        self.content = contentElement
        self.upperLeft = self.upperRight = None
        self.lowerLeft = self.lowerRight = None


    def elementAt(self, point, startPoint):
        r = BasicElement.elementAt(self, point, startPoint)
        if r != None:
            x, y = startPoint.x(), startPoint.y()
            r = self.content.elementAt(point, QPoint(x+self.content.x(),
                                                     y+self.content.y()))
            if r != None: return r

            if self.upperRight != None:
                r = self.upperRight.elementAt(point, QPoint(x+self.upperRight.x(),
                                                            y+self.upperRight.y()))
                if r != None: return r

            if self.upperLeft != None:
                r = self.upperLeft.elementAt(point, QPoint(x+self.upperLeft.x(),
                                                           y+self.upperLeft.y()))
                if r != None: return r

            if self.lowerRight != None:
                r = self.lowerRight.elementAt(point, QPoint(x+self.lowerRight.x(),
                                                            y+self.lowerRight.y()))
                if r != None: return r

            if self.lowerLeft != None:
                r = self.lowerLeft.elementAt(point, QPoint(x+self.lowerLeft.x(),
                                                           y+self.lowerLeft.y()))
                if r != None: return r

            return self
        
            
    def moveLeft(self, cursor, fromElement):
        assert fromElement != None

        if cursor.isSelection():
            self.parent.moveLeft(cursor, self)
            
        elif fromElement == self.parent:
            if self.lowerRight != None:
                self.lowerRight.moveLeft(cursor, self)
            elif self.upperRight != None:
                self.upperRight.moveLeft(cursor, self)
            else:
                self.content.moveLeft(cursor, self)

        elif fromElement == self.lowerRight:
            if self.upperRight != None:
                self.upperRight.moveLeft(cursor, self)
            else:
                self.content.moveLeft(cursor, self)

        elif fromElement == self.upperRight:
            self.content.moveLeft(cursor, self)

        elif fromElement == self.content:
            if self.lowerLeft != None:
                self.lowerLeft.moveLeft(cursor, self)
            elif self.upperLeft != None:
                self.upperLeft.moveLeft(cursor, self)
            else:
                self.parent.moveLeft(cursor, self)

        elif fromElement == self.lowerLeft:
            if self.upperLeft != None:
                self.upperLeft.moveLeft(cursor, self)
            else:
                self.parent.moveLeft(cursor, self)
                
        else:
            self.parent.moveLeft(cursor, self)
            
        
    def moveRight(self, cursor, fromElement):
        assert fromElement != None

        if cursor.isSelection():
            self.parent.moveRight(cursor, self)

        elif fromElement == self.parent:
            if self.upperLeft != None:
                self.upperLeft.moveRight(cursor, self)
            elif self.lowerLeft != None:
                self.lowerLeft.moveRight(cursor, self)
            else:
                self.content.moveRight(cursor, self)

        elif fromElement == self.upperLeft:
            if self.lowerLeft != None:
                self.lowerLeft.moveRight(cursor, self)
            else:
                self.content.moveRight(cursor, self)

        elif fromElement == self.lowerLeft:
            self.content.moveRight(cursor, self)

        elif fromElement == self.content:
            if self.upperRight != None:
                self.upperRight.moveRight(cursor, self)
            elif self.lowerRight != None:
                self.lowerRight.moveRight(cursor, self)
            else:
                self.parent.moveRight(cursor, self)

        elif fromElement == self.upperRight:
            if self.lowerRight != None:
                self.lowerRight.moveRight(cursor, self)
            else:
                self.parent.moveRight(cursor, self)
                
        else:
            self.parent.moveRight(cursor, self)


    def moveUp(self, cursor, fromElement):
        assert fromElement != None

        if fromElement == self.parent:
            self.content.moveRight(cursor, self)

        elif fromElement == self.upperLeft or fromElement == self.upperRight:
            self.parent.moveUp(cursor, self)

        elif fromElement == self.content:
            if self.upperRight != None:
                self.upperRight.moveRight(cursor, self)
            elif self.upperLeft != None:
                #self.upperLeft.moveRight(cursor, self)
                self.upperLeft.moveLeft(cursor, self)
            else:
                self.parent.moveUp(cursor, self)

        elif fromElement == self.lowerLeft:
            self.content.moveRight(cursor, self)

        elif fromElement == self.lowerRight:
            self.content.moveLeft(cursor, self)

        else: # should never happen.
            self.parent.moveUp(cursor, self)
                

    def moveDown(self, cursor, fromElement):
        assert fromElement != None

        if fromElement == self.parent:
            self.content.moveRight(cursor, self)

        elif fromElement == self.lowerLeft or fromElement == self.lowerRight:
            self.parent.moveDown(cursor, fromElement)

        elif fromElement == self.content:
            if self.lowerRight != None:
                self.lowerRight.moveRight(cursor, self)
            elif self.lowerLeft != None:
                #self.lowerLeft.moveRight(cursor, self)
                self.lowerLeft.moveLeft(cursor, self)
            else:
                self.parent.moveDown(cursor, self)

        elif fromElement == self.upperLeft:
            self.content.moveRight(cursor, self)

        elif fromElement == self.upperRight:
            self.content.moveLeft(cursor, self)
            
        else: # should never happen.
            self.parent.moveDown(cursor, self)

            
    def draw(self, painter, styleContext, startPoint):
        x, y = startPoint.x(), startPoint.y()
        self.content.draw(painter, styleContext,
                          QPoint(x+self.content.x(),
                                 y+self.content.y()))
        if self.upperLeft != None:
            self.upperLeft.draw(painter, styleContext,
                                QPoint(x+self.upperLeft.x(),
                                       y+self.upperLeft.y()))
        if self.upperRight != None:
            self.upperRight.draw(painter, styleContext,
                                 QPoint(x+self.upperRight.x(),
                                        y+self.upperRight.y()))
        if self.lowerLeft != None:
            self.lowerLeft.draw(painter, styleContext,
                                QPoint(x+self.lowerLeft.x(),
                                       y+self.lowerLeft.y()))
        if self.lowerRight != None:
            self.lowerRight.draw(painter, styleContext,
                                 QPoint(x+self.lowerRight.x(),
                                        y+self.lowerRight.y()))

        # Debug
        painter.setPen(Qt.red)
        painter.drawRect(x, y, self.width(), self.height())


    def calcSizes(self, styleContext):

        # get the indexes size
        if self.upperLeft != None:
            self.upperLeft.calcSizes(styleContext)
            ulWidth = self.upperLeft.width()
            ulHeight = self.upperLeft.height()
            ulMidline = self.upperLeft.midline
        else:
            ulWidth = ulHeight = ulMidline = 0

        if self.upperRight != None:
            self.upperRight.calcSizes(styleContext)
            urWidth = self.upperRight.width()
            urHeight = self.upperRight.height()
            urMidline = self.upperRight.midline
        else:
            urWidth = urHeight = urMidline = 0

        if self.lowerLeft != None:
            self.lowerLeft.calcSizes(styleContext)
            llWidth = self.lowerLeft.width()
            llHeight = self.lowerLeft.height()
            llMidline = self.lowerLeft.midline
        else:
            llWidth = llHeight = llMidline = 0

        if self.lowerRight != None:
            self.lowerRight.calcSizes(styleContext)
            lrWidth = self.lowerRight.width()
            lrHeight = self.lowerRight.height()
            lrMidline = self.lowerRight.midline
        else:
            lrWidth = lrHeight = lrMidline = 0

        # get the contents size
        self.content.calcSizes(styleContext)
        width = self.content.width()
        toMidline = self.content.midline
        fromMidline = self.content.height() - toMidline

        # calculate the x offsets
        if ulWidth > llWidth:
            self.upperLeft.setX(0)
            if self.lowerLeft != None:
                self.lowerLeft.setX(ulWidth - llWidth)
            self.content.setX(ulWidth)
            width += ulWidth
        else:
            if self.upperLeft != None:
                self.upperLeft.setX(llWidth - ulWidth)
            if self.lowerLeft != None:
                self.lowerLeft.setX(0)
            self.content.setX(llWidth)
            width += llWidth

        if self.upperRight != None:
            self.upperRight.setX(width)
        if self.lowerRight != None:
            self.lowerRight.setX(width)

        width += max(urWidth, lrWidth)
        
        # calculate the y offsets
        if ulHeight > urHeight:
            self.upperLeft.setY(0)
            if self.upperRight != None:
                self.upperRight.setY(ulHeight - urHeight)
            self.content.setY(max(ulHeight - toMidline/2, 0))
            toMidline += self.content.y()
        else:
            if self.upperLeft != None:
                self.upperLeft.setY(urHeight - ulHeight)
            if self.upperRight != None:
                self.upperRight.setY(0)
            self.content.setY(max(urHeight - toMidline/2, 0))
            toMidline += self.content.y()

        if self.lowerLeft != None:
            self.lowerLeft.setY(toMidline + fromMidline/2)
        if self.lowerRight != None:
            self.lowerRight.setY(toMidline + fromMidline/2)

        fromMidline += max(max(llHeight, lrHeight) - fromMidline/2, 0)

        # set the result
        self.setWidth(width)
        self.setHeight(toMidline+fromMidline)
        #self.midline = self.height()/2
        self.midline = toMidline


    def mainChild(self):
        return self.content

    def setMainChild(self, sequenceElement):
        self.content = sequenceElement
        self.content.parent = self
        self.formula().changed()

    def removeChild(self, cursor, element):
        if element == self.upperLeft:
            self.formula().elementRemoved(self.upperLeft)
            self.content.moveRight(cursor, self)
            self.upperLeft = None
        elif element == self.lowerLeft:
            self.formula().elementRemoved(self.lowerLeft)
            self.content.moveRight(cursor, self)
            self.lowerLeft = None
        elif element == self.upperRight:
            self.formula().elementRemoved(self.upperRight)
            self.content.moveLeft(cursor, self)
            self.upperRight = None
        elif element == self.lowerRight:
            self.formula().elementRemoved(self.lowerRight)
            self.content.moveLeft(cursor, self)
            self.lowerRight = None
        elif element == self.content:
            self.parent.removeChild(cursor, self)
            return

        if self.upperLeft == None and self.lowerLeft == None and \
           self.upperRight == None and self.lowerRight == None:
            self.parent.replaceElementByMainChild(cursor, self)
        else:
            self.formula().changed()
        

    def requireUpperLeft(self):
        if self.upperLeft == None:
            self.upperLeft = SequenceElement(self)
            self.formula().changed()
        return self.upperLeft

    def requireUpperRight(self):
        if self.upperRight == None:
            self.upperRight = SequenceElement(self)
            self.formula().changed()
        return self.upperRight

    def requireLowerLeft(self):
        if self.lowerLeft == None:
            self.lowerLeft = SequenceElement(self)
            self.formula().changed()
        return self.lowerLeft

    def requireLowerRight(self):
        if self.lowerRight == None:
            self.lowerRight = SequenceElement(self)
            self.formula().changed()
        return self.lowerRight
    
    
class Cursor:
    """The selection. This might be a one position selection or
    an area. Handles user input and object creation.
    
    Note that it is up to the elements to actually move the cursor.
    (The cursor has no chance to know how.)"""
    
    def __init__(self, formulaElement):
        self.sequenceElement = formulaElement
        self.currentPos = 0
        self.currentMarkPos = -1
        self.selectionFlag = 0
        self.mouseMarkFlag = 0

    def isSelection(self):
        return self.selectionFlag

    def mouseMark(self):
        return self.mouseMarkFlag

    def set(self, sequenceElement, pos):
        """Set the cursor to a new position."""
        if self.isSelection():
            if self.currentMarkPos == -1:
                self.currentMarkPos = self.currentPos
            if self.currentMarkPos == pos:
                self.selectionFlag = 0
        else:
            self.currentMarkPos = -1
        
        self.sequenceElement = sequenceElement
        self.currentPos = pos

    def markPos(self):
        return self.currentMarkPos
    
    def setMarkPos(self, markPos):
        """Gets called by elements if the cursor moves up to the parent."""
        self.selectionFlag = (markPos != -1)
        self.currentMarkPos = markPos
        
    def pos(self):
        return self.currentPos

    def element(self):
        return self.sequenceElement

    
    def draw(self, painter):
        point = self.sequenceElement.globalCursorPos(self.pos())
        height = self.sequenceElement.height()

        if self.isSelection():
            markPoint = self.sequenceElement.globalCursorPos(self.markPos())

            x = min(point.x(), markPoint.x())
            width = abs(point.x() - markPoint.x())
            painter.setRasterOp(Qt.XorROP)
            #painter.setRasterOp(Qt.OrROP)
            painter.fillRect(x, point.y(), width, height, QBrush(Qt.white))
            #painter.drawLine(point.x(), point.y()-2,
            #                 point.x(), point.y()+height+2)
            painter.setRasterOp(Qt.CopyROP)
        else:
            painter.setPen(Qt.blue)
            painter.drawLine(point.x(), point.y()-2,
                             point.x(), point.y()+height+2)

            

    def findIndexElement(self):
        """Looks if we are just behind an IndexElement or at the last
        position of an IndexElement's content and returns the element then."""
        element = self.sequenceElement.elementAtCursor(self)
        if isinstance(element, IndexElement):
            return element
        if self.pos() == self.sequenceElement.countChildren():
            parent = self.sequenceElement.parent
            if isinstance(parent, IndexElement):
                if self.sequenceElement == parent.mainChild():
                    return parent

                
    def addUpperRightIndex(self):
        indexElement = self.findIndexElement()
        if indexElement == None:
            indexElement = IndexElement(None)
            self.sequenceElement.replaceCurrentSelection(self, indexElement)
        index = indexElement.requireUpperRight()

        index.moveRight(self, index.parent)

    
    def addLowerRightIndex(self):
        indexElement = self.findIndexElement()
        if indexElement == None:
            indexElement = IndexElement(None)
            self.sequenceElement.replaceCurrentSelection(self, indexElement)
        index = indexElement.requireLowerRight()

        index.moveRight(self, index.parent)
        

    def addTextElement(self, char):
        textElement = TextElement(self.sequenceElement, QString(char))
        self.sequenceElement.insertChild(self, textElement)
    

    def handleKey(self, keyEvent):
        action = keyEvent.key()
        state = keyEvent.state()
        char = keyEvent.text().at(0)
        
        self.mouseMarkFlag = 0
        
        if char.isPrint():
            #self.sequenceElement.handleKey(self, char)
            latin1 = char.latin1()
            if latin1 == '[':
                #addBracketElement("[]")
                pass
            elif latin1 == '(':
                #addBracketElement(DEFAULT_DELIMITER)
                pass
            elif latin1 == '|':
                #addBracketElement("||")
                pass
            elif latin1 == '/':
                #addFractionElement(DEFAULT_FRACTION)
                pass
            elif latin1 == '@':
                #addRootElement()
                pass
            elif latin1 == '^':
                self.addUpperRightIndex()
            elif latin1 == '_':
                self.addLowerRightIndex()
            elif latin1 == ' ':
                # no space allowed.
                pass
            else:
                self.addTextElement(char)
            
        else:

            if Qt.Key_BackSpace == action:
                self.sequenceElement.removeChildBefore(self)
                return
            elif Qt.Key_Delete == action:
                self.sequenceElement.removeChildAt(self)
                return

            self.selectionFlag = state & Qt.ShiftButton
            if Qt.Key_Left == action:
                if state & Qt.ControlButton:
                    self.sequenceElement.moveHome(self)
                else:
                    self.sequenceElement.moveLeft(self, self.sequenceElement)
            elif Qt.Key_Right == action:
                if state & Qt.ControlButton:
                    self.sequenceElement.moveEnd(self)
                else:
                    self.sequenceElement.moveRight(self, self.sequenceElement)
            elif Qt.Key_Up == action:
                self.sequenceElement.moveUp(self, self.sequenceElement)
            elif Qt.Key_Down == action:
                self.sequenceElement.moveDown(self, self.sequenceElement)
            elif Qt.Key_Home == action:
                self.sequenceElement.formula().moveHome(self)
            elif Qt.Key_End == action:
                self.sequenceElement.formula().moveEnd(self)

        # Qt.Key_PageUp, Qt.Key_PageDown,
        

    def handleMousePress(self, mouseEvent):
        formula = self.sequenceElement.formula()
        element = formula.elementAt(mouseEvent.pos(), QPoint(0, 0))
        if element != None:
            if element.parent != None:
                element.moveLeft(self, element.parent)
                self.selectionFlag = 0
                self.mouseMarkFlag = 1
                self.setMarkPos(self.pos())
            #else:
            #    self.set(formula, 0)

    def handleMouseRelease(self, mouseEvent):
        self.mouseMarkFlag = 0
    
    def handleMouseMove(self, mouseEvent):
        self.selectionFlag = 1
        formula = self.sequenceElement.formula()
        element = formula.elementAt(mouseEvent.pos(), QPoint(0, 0))
        if element != None:
            if element.parent != None:
                element.parent.moveLeft(self, element)
    

    def elementRemoved(self, element):
        """The cursor must not be inside a leaf which gets cut off.
        We assume the FormulaElement will never be removed."""
        e = self.sequenceElement
        while e != None:
            if e == element:
                # This is meant to catch all cursors that did not
                # cause the deletion.
                e.parent.moveRight(self, e)
                self.sequenceElement.moveHome(self)
                return
            e = e.parent
        

class StyleContext:
    """Contains all variable information that are needed to
    draw a formula."""

    def __init__(self):
        self.font = QFont("helvetica", 18)

    def setupPainter(self, painter):
        painter.setFont(self.font)
        painter.setPen(Qt.black)

    def fontMetrics(self):
        return QFontMetrics(self.font)
    
    
class Widget(QWidget):
    """The widget that contains a formula."""
    
    def __init__(self):
        QWidget.__init__(self)
        f = self.formula = FormulaElement(self)
        self.cursor = Cursor(self.formula)
        self.styleContext = StyleContext()

        # Test data
        f.addChild(TextElement(f, "y"))
        f.addChild(TextElement(f, "="))

        s1 = SequenceElement(f)
        s1.addChild(TextElement(s1, "e"))

        i1 = IndexElement(s1)
        f.addChild(i1)

        s2 = i1.requireUpperRight()
        s2.addChild(TextElement(s2, "-"))
        s2.addChild(TextElement(s2, "o"))
        s2.addChild(TextElement(s2, "t"))

        f.addChild(TextElement(f, "("))
        f.addChild(TextElement(f, "s"))
        f.addChild(TextElement(f, "i"))
        f.addChild(TextElement(f, "n"))
        f.addChild(TextElement(f, "("))
        f.addChild(TextElement(f, "x"))
        f.addChild(TextElement(f, ")"))
        f.addChild(TextElement(f, ")"))

        s3 = SequenceElement(f)
        s3.addChild(TextElement(s3, "+"))
        s3.addChild(TextElement(s3, "f"))
        s3.addChild(TextElement(s3, "u"))
        s3.addChild(TextElement(s3, "n"))
        
        i2 = IndexElement(s3)
        i2.requireUpperLeft()
        i2.requireUpperRight()
        i2.requireLowerLeft()
        i2.requireLowerRight()
        
        f.addChild(i2)

        f.addChild(TextElement(f, ":"))
        f.addChild(TextElement(f, "-"))
        f.addChild(TextElement(f, ")"))

        s4 = SequenceElement(f)
        s4.addChild(TextElement(s4, "#"))

        i3 = IndexElement(s4)

        s5 = i3.requireUpperLeft()
        s5.addChild(TextElement(s5, "u"))
        s6 = i3.requireLowerLeft()
        s6.addChild(TextElement(s6, "d"))

        f.addChild(i3)

        self.changedFlag = 1

        
    def changed(self):
        """Gets called each time the formula changes."""
        self.changedFlag = 1
        
        
    def elementRemoved(self, element):
        """The element is going to go real soon."""
        self.cursor.elementRemoved(element)
        
        
    def paintEvent (self, e):

        if self.changedFlag:
            # You need to use the same StyleContext you use for drawing.
            self.formula.calcSizes(self.styleContext)
            self.changedFlag = 0
            
        painter = QPainter()
        painter.begin(self)
        try:
            self.formula.draw(painter, self.styleContext, QPoint(0, 0))
            self.cursor.draw(painter)
        finally:
            painter.end()


    def keyPressEvent(self, e):
        self.cursor.handleKey(e)
        self.update()
    
    def mousePressEvent(self, e):
        self.cursor.handleMousePress(e)
        self.update()
        
    def mouseReleaseEvent(self, e):
        self.cursor.handleMouseRelease(e)
        self.update()
    
    def mouseDoubleClickEvent(self, e):
        pass

    def mouseMoveEvent(self, e):
        self.cursor.handleMouseMove(e)
        self.update()
    
