"""
SPDX-FileCopyrightText: 2017 Eliakin Costa <eliakim170@gmail.com>

SPDX-License-Identifier: GPL-2.0-or-later
"""
from PyQt5.QtCore import Qt, QRect, QSize, QPoint, pyqtSlot
from PyQt5.QtWidgets import QPlainTextEdit, QTextEdit, QLabel
from PyQt5.QtGui import QIcon, QColor, QPainter, QTextFormat, QFont, QTextCursor
from scripter.ui_scripter.editor import linenumberarea, debugarea

##################
# Constants
##################

INDENT_WIDTH = 4  # size in spaces of indent in editor window.
# ideally make this a setting sometime?

MODIFIER_COMMENT = Qt.ControlModifier
KEY_COMMENT = Qt.Key_M
CHAR_COMMENT = "#"

CHAR_SPACE = " "
CHAR_COLON = ":"
CHAR_COMMA = ","
CHAR_CONTINUATION = "\\"
CHAR_OPEN_BRACKET = "("
CHAR_OPEN_SQUARE_BRACKET = "["
CHAR_OPEN_BRACE = "{"
CHAR_EQUALS = "="


class CodeEditor(QPlainTextEdit):

    DEBUG_AREA_WIDTH = 20

    def __init__(self, scripter, parent=None):
        super(CodeEditor, self).__init__(parent)

        self.setLineWrapMode(self.NoWrap)

        self.scripter = scripter
        self.lineNumberArea = linenumberarea.LineNumberArea(self)
        self.debugArea = debugarea.DebugArea(self)

        self.blockCountChanged.connect(self.updateMarginsWidth)
        self.updateRequest.connect(self.updateLineNumberArea)
        self.cursorPositionChanged.connect(self.highlightCurrentLine)

        self.updateMarginsWidth()
        self.highlightCurrentLine()
        self.font = "Monospace"
        self._stepped = False
        self.debugArrow = QIcon(':/icons/debug_arrow.svg')
        self.setCornerWidget(QLabel(str()))
        self._documentChanged = False
        self.indent_width = INDENT_WIDTH  # maybe one day connect this to a setting

        self.undoAvailable.connect(self.setDocumentModified)

    def debugAreaWidth(self):
        return self.DEBUG_AREA_WIDTH

    def lineNumberAreaWidth(self):
        """The lineNumberAreaWidth is the quatity of decimal places in blockCount"""
        digits = 1
        max_ = max(1, self.blockCount())
        while (max_ >= 10):
            max_ /= 10
            digits += 1

        space = 3 + self.fontMetrics().width('9') * digits + 3

        return space

    def resizeEvent(self, event):
        super(CodeEditor, self).resizeEvent(event)

        qRect = self.contentsRect()
        self.debugArea.setGeometry(QRect(qRect.left(),
                                         qRect.top(),
                                         self.debugAreaWidth(),
                                         qRect.height()))
        scrollBarHeight = 0
        if (self.horizontalScrollBar().isVisible()):
            scrollBarHeight = self.horizontalScrollBar().height()

        self.lineNumberArea.setGeometry(QRect(qRect.left() + self.debugAreaWidth(),
                                              qRect.top(),
                                              self.lineNumberAreaWidth(),
                                              qRect.height() - scrollBarHeight))

    def updateMarginsWidth(self):
        self.setViewportMargins(self.lineNumberAreaWidth() + self.debugAreaWidth(), 0, 0, 0)

    def updateLineNumberArea(self, rect, dy):
        """ This slot is invoked when the editors viewport has been scrolled """

        if dy:
            self.lineNumberArea.scroll(0, dy)
            self.debugArea.scroll(0, dy)
        else:
            self.lineNumberArea.update(0, rect.y(), self.lineNumberArea.width(), rect.height())

        if rect.contains(self.viewport().rect()):
            self.updateMarginsWidth()

    def lineNumberAreaPaintEvent(self, event):
        """This method draws the current lineNumberArea for while"""
        blockColor = QColor(self.palette().base().color()).darker(120)
        if (self.palette().base().color().lightness() < 128):
            blockColor = QColor(self.palette().base().color()).lighter(120)
        if (self.palette().base().color().lightness() < 1):
            blockColor = QColor(43, 43, 43)
        painter = QPainter(self.lineNumberArea)
        painter.fillRect(event.rect(), blockColor)

        block = self.firstVisibleBlock()
        blockNumber = block.blockNumber()
        top = int(self.blockBoundingGeometry(block).translated(self.contentOffset()).top())
        bottom = top + int(self.blockBoundingRect(block).height())
        while block.isValid() and top <= event.rect().bottom():
            if block.isVisible() and bottom >= event.rect().top():
                number = str(blockNumber + 1)
                painter.setPen(self.palette().text().color())
                painter.drawText(0, top, self.lineNumberArea.width() - 3, self.fontMetrics().height(),
                                 Qt.AlignRight, number)

            block = block.next()
            top = bottom
            bottom = top + int(self.blockBoundingRect(block).height())
            blockNumber += 1

    def debugAreaPaintEvent(self, event):
        if self.scripter.debugcontroller.isActive and self.scripter.debugcontroller.currentLine:
            lineNumber = self.scripter.debugcontroller.currentLine
            block = self.document().findBlockByLineNumber(lineNumber - 1)

            if self._stepped:
                cursor = QTextCursor(block)
                self.setTextCursor(cursor)
                self._stepped = False

            top = int(self.blockBoundingGeometry(block).translated(self.contentOffset()).top())

            painter = QPainter(self.debugArea)
            pixmap = self.debugArrow.pixmap(QSize(self.debugAreaWidth() - 3, int(self.blockBoundingRect(block).height())))
            painter.drawPixmap(QPoint(0, top), pixmap)

    def highlightCurrentLine(self):
        """Highlight current line under cursor"""
        currentSelection = QTextEdit.ExtraSelection()

        lineColor = QColor(self.palette().base().color()).darker(120)
        if (self.palette().base().color().lightness() < 128):
            lineColor = QColor(self.palette().base().color()).lighter(120)
        if (self.palette().base().color().lightness() < 1):
            lineColor = QColor(43, 43, 43)

        currentSelection.format.setBackground(lineColor)
        currentSelection.format.setProperty(QTextFormat.FullWidthSelection, True)
        currentSelection.cursor = self.textCursor()
        currentSelection.cursor.clearSelection()

        self.setExtraSelections([currentSelection])

    def wheelEvent(self, e):
        """When the CTRL is pressed during the wheelEvent, zoomIn and zoomOut
           slots are invoked"""
        if e.modifiers() == Qt.ControlModifier:
            delta = e.angleDelta().y()
            if delta < 0:
                self.zoomOut()
            elif delta > 0:
                self.zoomIn()
        else:
            super(CodeEditor, self).wheelEvent(e)

    def keyPressEvent(self, e):
        modifiers = e.modifiers()
        if (e.key() == Qt.Key_Tab):
            self.indent()
        elif e.key() == Qt.Key_Backtab:
            self.dedent()
        elif modifiers == MODIFIER_COMMENT and e.key() == KEY_COMMENT:
            self.toggleComment()
        elif e.key() == Qt.Key_Return:
            super(CodeEditor, self).keyPressEvent(e)
            self.autoindent()
        else:
            super(CodeEditor, self).keyPressEvent(e)

    def isEmptyBlock(self, blockNumber):
        """ test whether block with number blockNumber contains any non-whitespace
        If only whitespace: return true, else return false"""

        # get block text
        cursor = self.textCursor()
        cursor.movePosition(QTextCursor.Start)
        cursor.movePosition(QTextCursor.NextBlock, n=blockNumber)
        cursor.movePosition(QTextCursor.StartOfLine)
        cursor.movePosition(QTextCursor.EndOfLine, QTextCursor.KeepAnchor)
        text = cursor.selectedText()
        if text.strip() == "":
            return True
        else:
            return False

    def indent(self):
        # tab key has been pressed. Indent current line or selected block by self.indent_width

        cursor = self.textCursor()
        # is there a selection?

        selectionStart = cursor.selectionStart()
        selectionEnd = cursor.selectionEnd()

        if selectionStart == selectionEnd and cursor.atBlockEnd():
            # ie no selection and don't insert in the middle of text
            # something smarter might skip whitespace and add a tab in front of
            # the next non whitespace character
            cursor.insertText(" " * self.indent_width)
            return

        cursor.setPosition(selectionStart)
        startBlock = cursor.blockNumber()
        cursor.setPosition(selectionEnd)
        endBlock = cursor.blockNumber()

        cursor.movePosition(QTextCursor.Start)
        cursor.movePosition(QTextCursor.NextBlock, n=startBlock)

        for i in range(0, endBlock - startBlock + 1):
            if not self.isEmptyBlock(startBlock + i):  # Don't insert whitespace on empty lines
                cursor.movePosition(QTextCursor.StartOfLine)
                cursor.insertText(" " * self.indent_width)

            cursor.movePosition(QTextCursor.NextBlock)

        # QT maintains separate cursors, so don't need to track or reset user's cursor

    def dedentBlock(self, blockNumber):
        # dedent the line at blockNumber
        cursor = self.textCursor()
        cursor.movePosition(QTextCursor.Start)
        cursor.movePosition(QTextCursor.NextBlock, n=blockNumber)

        for _ in range(self.indent_width):
            cursor.movePosition(QTextCursor.StartOfLine)
            cursor.movePosition(QTextCursor.Right, QTextCursor.KeepAnchor)
            if cursor.selectedText() == " ":  # need to test each char
                cursor.removeSelectedText()
            else:
                break  # stop deleting!

        return

    def dedent(self):
        cursor = self.textCursor()
        selectionStart = cursor.selectionStart()
        selectionEnd = cursor.selectionEnd()

        cursor.setPosition(selectionStart)
        startBlock = cursor.blockNumber()
        cursor.setPosition(selectionEnd)
        endBlock = cursor.blockNumber()

        if endBlock < startBlock:
            startBlock, endBlock = endBlock, startBlock

        for blockNumber in range(startBlock, endBlock + 1):
            self.dedentBlock(blockNumber)

    def autoindent(self):
        """The return key has just been pressed (and processed by the editor)
        now insert leading spaces to reflect an appropriate indent level
        against the previous line.
        This will depend on the end of the previous line. If it ends:
        * with a colon (:) then indent to a new indent level
        * with a comma (,) then this is an implied continuation line, probably
          in the middle of a function's parameter list
          - look for last open bracket on previous line (, [ or {
            - if found indent to that level + one character,
            - otherwise use previous line whitespace, this is probably a list or
              parameter list so line up with other elements
        * with a backslash (\) then this is a continuation line, probably
          on the RHS of an assignment
          - similar rules as for comma, but if there is an = character
            use that plus one indent level if that is greater
        * if it is an open bracket of some sort treat similarly to comma


        * anything else - a new line at the same indent level. This will preserve
          the indent level of whitespace lines. User can shift-tab to dedent
          as necessary
        """

        cursor = self.textCursor()
        block = cursor.block()
        block = block.previous()
        text = block.text()
        indentLevel = len(text) - len(text.lstrip())  # base indent level

        # get last char
        try:
            lastChar = text.rstrip()[-1]
        except IndexError:
            lastChar = None

        # work out indent level
        if lastChar == CHAR_COLON:
            indentLevel = indentLevel + self.indent_width
        elif lastChar == CHAR_COMMA:  # technically these are mutually exclusive so if would work
            braceLevels = []
            for c in [CHAR_OPEN_BRACE, CHAR_OPEN_BRACKET, CHAR_OPEN_SQUARE_BRACKET]:
                braceLevels.append(text.rfind(c))
            bracePosition = max(braceLevels)
            if bracePosition > 0:
                indentLevel = bracePosition + 1
        elif lastChar == CHAR_CONTINUATION:
            braceLevels = []
            for c in [CHAR_OPEN_BRACE, CHAR_OPEN_BRACKET, CHAR_OPEN_SQUARE_BRACKET]:
                braceLevels.append(text.rfind(c))
            bracePosition = max(braceLevels)
            equalPosition = text.rfind(CHAR_EQUALS)
            if bracePosition > equalPosition:
                indentLevel = bracePosition + 1
            if equalPosition > bracePosition:
                indentLevel = equalPosition + self.indent_width
            # otherwise they're the same - ie both -1 so use base indent level
        elif lastChar in [CHAR_OPEN_BRACE, CHAR_OPEN_BRACKET, CHAR_OPEN_SQUARE_BRACKET]:
            indentLevel = len(text.rstrip())

        # indent
        cursor.insertText(CHAR_SPACE * indentLevel)

    def toggleComment(self):
        """Toggle lines of selected text to/from either comment or uncomment
        selected text is obtained from text cursor
        If selected text contains both commented and uncommented text this will
        flip the state of each line - which may not be desirable.
        """

        cursor = self.textCursor()
        selectionStart = cursor.selectionStart()
        selectionEnd = cursor.selectionEnd()

        cursor.setPosition(selectionStart)
        startBlock = cursor.blockNumber()
        cursor.setPosition(selectionEnd)
        endBlock = cursor.blockNumber()

        cursor.movePosition(QTextCursor.Start)
        cursor.movePosition(QTextCursor.NextBlock, n=startBlock)

        for _ in range(0, endBlock - startBlock + 1):
            # Test for empty line (if the line is empty moving the cursor right will overflow
            # to next line, throwing the line tracking off)
            cursor.movePosition(QTextCursor.StartOfLine)
            p1 = cursor.position()
            cursor.movePosition(QTextCursor.EndOfLine)
            p2 = cursor.position()
            if p1 == p2:  # empty line - comment it
                cursor.movePosition(QTextCursor.StartOfLine)
                cursor.insertText(CHAR_COMMENT)
                cursor.movePosition(QTextCursor.NextBlock)
                continue

            cursor.movePosition(QTextCursor.StartOfLine)
            cursor.movePosition(QTextCursor.Right, QTextCursor.KeepAnchor)
            text = cursor.selectedText()

            if text == CHAR_COMMENT:
                cursor.removeSelectedText()
            else:
                cursor.movePosition(QTextCursor.StartOfLine)
                cursor.insertText(CHAR_COMMENT)

            cursor.movePosition(QTextCursor.NextBlock)

    @property
    def font(self):
        return self._font

    @font.setter
    def font(self, font="Monospace"):
        self._font = font
        self.setFont(QFont(font, self.fontInfo().pointSize()))

    def setFontSize(self, size=10):
        self.setFont(QFont(self._font, size))

    def setStepped(self, status):
        self._stepped = status

    def repaintDebugArea(self):
        self.debugArea.repaint()

    @pyqtSlot(bool)
    def setDocumentModified(self, changed=False):
        self._documentModified = changed
