# -*- coding: utf-8 -*-

from PyQt5.QtCore import *
from PyQt5.QtWidgets import *
from scripter import linenumberarea
from PyQt5.QtGui import *

class CodeEditor(QPlainTextEdit):

    def __init__(self, parent=None):
        super(CodeEditor, self).__init__(parent)

        self.setLineWrapMode(self.NoWrap)

        self.lineNumberArea = linenumberarea.LineNumberArea(self)

        self.blockCountChanged.connect(self.updateLineNumberAreaWidth)
        self.updateRequest.connect(self.updateLineNumberArea)
        self.cursorPositionChanged.connect(self.highlightCurrentLine)

        self.updateLineNumberAreaWidth()
        self.highlightCurrentLine()

    def lineNumberAreaWidth(self):
        """The lineNumberAreaWidth is the quatity of decimal places in blockCount"""
        digits = 1
        max_ = max(1, self.blockCount())
        while (max_ >= 10):
            max_ /= 10
            digits += 1

        space = 3 + self.fontMetrics().width('9') * digits

        return space

    def resizeEvent(self, event):
          super(CodeEditor, self).resizeEvent(event)

          qRect = self.contentsRect();
          self.lineNumberArea.setGeometry(QRect(qRect.left(), qRect.top(), self.lineNumberAreaWidth(), qRect.height()));

    def updateLineNumberAreaWidth(self):
        self.setViewportMargins(self.lineNumberAreaWidth(), 0, 0, 0)

    def updateLineNumberArea(self, rect, dy):
        """ This slot is invoked when the editors viewport has been scrolled """

        if dy:
            self.lineNumberArea.scroll(0, dy)
        else:
            self.lineNumberArea.update(0, rect.y(), self.lineNumberArea.width(), rect.height())

        if rect.contains(self.viewport().rect()):
            self.updateLineNumberAreaWidth()

    def lineNumberAreaPaintEvent(self, event):
        """This method draws the current lineNumberArea for while"""
        painter = QPainter(self.lineNumberArea)
        painter.fillRect(event.rect(), Qt.lightGray)

        block = self.firstVisibleBlock()
        blockNumber = block.blockNumber()
        top = int(self.blockBoundingGeometry(block).translated(self.contentOffset()).top())
        bottom = top + int(self.blockBoundingRect(block).height())

        while block.isValid() and top <= event.rect().bottom():
            if block.isVisible() and bottom >= event.rect().top():
                number = str(blockNumber + 1)
                painter.setPen(Qt.black)
                painter.drawText(0, top, self.lineNumberArea.width(), self.fontMetrics().height(),
                                 Qt.AlignRight, number)

            block = block.next()
            top = bottom
            bottom = top + int(self.blockBoundingRect(block).height())
            blockNumber += 1

    def highlightCurrentLine(self):
        """Highlight current line under cursor"""
        currentSelection = QTextEdit.ExtraSelection()

        lineColor = QColor(Qt.gray).lighter(180)
        currentSelection.format.setBackground(lineColor)
        currentSelection.format.setProperty(QTextFormat.FullWidthSelection, True)
        currentSelection.cursor = self.textCursor()
        currentSelection.cursor.clearSelection()
        self.setExtraSelections([currentSelection])
