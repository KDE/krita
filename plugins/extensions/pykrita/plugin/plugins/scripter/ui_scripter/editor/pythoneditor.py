from PyQt5.QtCore import Qt, QRect, QSize, QPoint, pyqtSlot
from PyQt5.QtWidgets import QPlainTextEdit, QTextEdit, QLabel
from PyQt5.QtGui import QIcon, QColor, QPainter, QTextFormat, QFont, QTextCursor
from scripter.ui_scripter.editor import linenumberarea, debugarea


INDENT_WIDTH = 4 # size in spaces of indent in editor window.
# ideally make this a setting sometime?


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
        self.indent_width = INDENT_WIDTH # maybe one day connect this to a setting
        
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
                                              qRect.height()-scrollBarHeight))

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
        if (self.palette().base().color().lightness()<128):
            blockColor = QColor(self.palette().base().color()).lighter(120)
        if (self.palette().base().color().lightness()<1):
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
                painter.drawText(0, top, self.lineNumberArea.width()-3, self.fontMetrics().height(),
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
            bottom = top + int(self.blockBoundingRect(block).height())

            painter = QPainter(self.debugArea)
            pixmap = self.debugArrow.pixmap(QSize(self.debugAreaWidth() - 3, int(self.blockBoundingRect(block).height())))
            painter.drawPixmap(QPoint(0, top), pixmap)

    def highlightCurrentLine(self):
        """Highlight current line under cursor"""
        currentSelection = QTextEdit.ExtraSelection()

        lineColor = QColor(self.palette().base().color()).darker(120)
        if (self.palette().base().color().lightness()<128):
            lineColor = QColor(self.palette().base().color()).lighter(120)
        if (self.palette().base().color().lightness()<1):
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
        if (e.key() == Qt.Key_Tab):
            self.indent()
        elif e.key() == Qt.Key_Backtab:
            self.dedent()
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
            cursor.insertText(" "*self.indent_width)
            return
            
        cursor.setPosition(selectionStart)
        startBlock = cursor.blockNumber()
        cursor.setPosition(selectionEnd)
        endBlock = cursor.blockNumber()

        cursor.movePosition(QTextCursor.Start)        
        cursor.movePosition(QTextCursor.NextBlock,n=startBlock) 
        
        for i in range(0, endBlock-startBlock+1):
            if not self.isEmptyBlock(startBlock+i): # Don't insert whitespace on empty lines
                cursor.movePosition(QTextCursor.StartOfLine)
                cursor.insertText(" "*self.indent_width)  
            
            cursor.movePosition(QTextCursor.NextBlock)
               
        # QT maintains separate cursors, so don't need to track or reset user's cursor


    def dedentBlock(self, blockNumber):
        # dedent the line at blockNumber  
        cursor = self.textCursor()
        cursor.movePosition(QTextCursor.Start)
        cursor.movePosition(QTextCursor.NextBlock, n=blockNumber) 

        for i in range(self.indent_width):
            cursor.movePosition(QTextCursor.StartOfLine)
            cursor.movePosition(QTextCursor.Right, QTextCursor.KeepAnchor)
            if cursor.selectedText() == " ": # need to test each char
                cursor.removeSelectedText()
            else:
                break # stop deleting!
            
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
        
        for blockNumber in range(startBlock, endBlock+1):
            self.dedentBlock(blockNumber)
        
         
    @property
    def font(self):
        return self._font

    @font.setter
    def font(self, font = "Monospace"):
        self._font = font
        self.setFont(QFont(font, self.fontInfo().pointSize()))
    
    def setFontSize(self, size=10):
        self.setFont( QFont( self._font, size) )

    def setStepped(self, status):
        self._stepped = status

    def repaintDebugArea(self):
        self.debugArea.repaint()

    @pyqtSlot(bool)
    def setDocumentModified(self, changed = False):
        self._documentModified = changed
