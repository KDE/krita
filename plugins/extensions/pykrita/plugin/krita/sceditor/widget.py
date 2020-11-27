# -*- coding: utf-8 -*-
#
#  SPDX-License-Identifier: GPL-3.0-or-later
#

import re
import sys
import os

# I put the rope package into a ZIP-file to save space
# and to keep everything clear
path = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(path, "rope.zip"))


from rope.base.project import get_no_project
from rope.contrib.codeassist import code_assist

from PyQt5.QtCore import QCoreApplication, QLine, Qt
from PyQt5.QtGui import (QBrush, QColor, QFont, QKeyEvent, QTextBlockUserData,
                         QTextCursor, QPainter, QPalette, QPen)
from PyQt5.QtWidgets import (QApplication, QFrame, QHBoxLayout, QMessageBox,
                             QPlainTextEdit, QVBoxLayout, QWidget)

from indenter import PythonCodeIndenter
from assist import AutoComplete, CallTip

from highlighter import PythonHighlighter,  QtQmlHighlighter


class EditorBlockData(QTextBlockUserData):

    def __init__(self):
        QTextBlockUserData.__init__(self)


class RopeEditorWrapper(object):

    def __init__(self, editview):
        self.editview = editview

    def length(self):
        return self.editview.length()

    def line_editor(self):
        return self

    def _get_block(self, line_no=None):
        cursor = self.editview.textCursor()
        row = cursor.blockNumber()
        if line_no == None:
            line_no = row
        block = cursor.block()
        while row > line_no:
            block = block.previous()
            row -= 1
        while row < line_no:
            block = block.next()
            row += 1
        return block

    def get_line(self, line_no=None):
        return unicode(self._get_block(line_no).text())

    def indent_line(self, line_no, indent_length):
        block = self._get_block(line_no)
        cursor = QTextCursor(block)
        cursor.joinPreviousEditBlock()
        cursor.movePosition(QTextCursor.StartOfBlock, QTextCursor.MoveAnchor)
        if indent_length < 0:
            for i in range(-indent_length):
                cursor.deleteChar()
        else:
            cursor.insertText(" " * indent_length)
        if indent_length:
            cursor.movePosition(
                QTextCursor.StartOfBlock, QTextCursor.MoveAnchor)
            line = unicode(cursor.block().text())
            if len(line) and line[0] == " ":
                cursor.movePosition(
                    QTextCursor.NextWord, QTextCursor.MoveAnchor)
            self.editview.setTextCursor(cursor)
        cursor.endEditBlock()


class EditorView(QPlainTextEdit):

    def __init__(self, parent=None, text=None,
                 EditorHighlighterClass=PythonHighlighter,
                 indenter=PythonCodeIndenter):
        QPlainTextEdit.__init__(self, parent)
        self.setFrameStyle(QFrame.NoFrame)
        self.setTabStopWidth(4)
        self.setLineWrapMode(QPlainTextEdit.NoWrap)
        font = QFont()
        font.setFamily("lucidasanstypewriter")
        font.setFixedPitch(True)
        font.setPointSize(10)
        self.setFont(font)
        self.highlighter = EditorHighlighterClass(self)
        if text:
            self.setPlainText(text)
        self.frame_style = self.frameStyle()
        self.draw_line = True
        self.print_width = self.fontMetrics().width("x" * 78)
        self.line_pen = QPen(QColor("lightgrey"))
        self.last_row = self.last_col = -1
        self.last_block = None
        self.highlight_line = True
        self.highlight_color = self.palette().highlight().color().light(175)
        self.highlight_brush = QBrush(QColor(self.highlight_color))
        self.cursorPositionChanged.connect(self.onCursorPositionChanged)
        self.indenter = indenter(RopeEditorWrapper(self))
        # True if you want to catch Emacs keys in actions
        self.disable_shortcuts = False

        self.prj = get_no_project()
        self.prj.root = None
        self.calltip = CallTip(self)
        self.autocomplete = AutoComplete(self)

    def closeEvent(self, event):
        self.calltip.close()
        self.autocomplete.close()

    def isModified(self):
        return self.document().isModified()

    def setModified(self, flag):
        self.document().setModified(flag)

    def length(self):
        return self.document().blockCount()

    def goto(self, line_no):
        cursor = self.textCursor()
        block = cursor.block()
        row = cursor.blockNumber()
        while row > line_no:
            block = block.previous()
            row -= 1
        while row < line_no:
            block = block.next()
            row += 1
        cursor = QTextCursor(block)
        self.setTextCursor(cursor)

    def move_start_of_doc(self):
        cursor = self.textCursor()
        cursor.setPosition(0)
        self.setTextCursor(cursor)

    def move_end_of_doc(self):
        cursor = self.textCursor()
        block = cursor.block()
        while block.isValid():
            last_block = block
            block = block.next()
        cursor.setPosition(last_block.position())
        cursor.movePosition(
            QTextCursor.EndOfBlock, QTextCursor.MoveAnchor)
        self.setTextCursor(cursor)

    def move_start_of_row(self):
        cursor = self.textCursor()
        cursor.movePosition(
            QTextCursor.StartOfBlock, QTextCursor.MoveAnchor)
        self.setTextCursor(cursor)

    def move_end_of_row(self):
        cursor = self.textCursor()
        cursor.movePosition(
            QTextCursor.EndOfBlock, QTextCursor.MoveAnchor)
        self.setTextCursor(cursor)

    def highline(self, cursor):
        self.viewport().update()

    def onCursorPositionChanged(self):
        cursor = self.textCursor()
        row, col = cursor.blockNumber(), cursor.columnNumber()
        if self.last_row != row:
            self.last_row = row
            if self.highlight_line:
                self.highline(cursor)
        if col != self.last_col:
            self.last_col = col
        self.cursorPositionChanged.emit(row, col)

    def _create_line(self):
        x = self.print_width
        self.line = QLine(x, 0, x, self.height())

    def resizeEvent(self, event):
        self._create_line()
        QPlainTextEdit.resizeEvent(self, event)

    def paintEvent(self, event):
        painter = QPainter(self.viewport())
        if self.highlight_line:
            r = self.cursorRect()
            r.setX(0)
            r.setWidth(self.viewport().width())
            painter.fillRect(r, self.highlight_brush)
        if self.draw_line:
            painter.setPen(self.line_pen)
            painter.drawLine(self.line)
        painter.end()
        QPlainTextEdit.paintEvent(self, event)

    def setDocument(self, document):
        QPlainTextEdit.setDocument(self, document)
        self.highlighter.setDocument(document)

    def indent(self):
        self.indenter.correct_indentation(self.textCursor().blockNumber())

    def tab_pressed(self):
        self.indent()

    def dedent(self):
        self.indenter.deindent(self.textCursor().blockNumber())

    def backtab_pressed(self):
        self.dedent()
        return True

    def backspace_pressed(self):
        cursor = self.textCursor()
        text = unicode(cursor.block().text())
        col = cursor.columnNumber()
        if col > 0 and text[:col].strip() == "":
            self.indenter.deindent(self.textCursor().blockNumber())
            return True

    def autocomplete_pressed(self):
        try:
            items = code_assist(self.prj,
                                unicode(self.toPlainText()),
                                self.textCursor().position())
        except Exception as e:
            items = []
        if items:
            self.autocomplete.setItems(items)
            self.autocomplete.show()

    def after_return_pressed(self):
        self.indenter.entering_new_line(self.textCursor().blockNumber())

    def keyPressEvent(self, event):
        if self.autocomplete.active:
            if self.autocomplete.keyPressEvent(event):
                return
        elif self.calltip.active:
            if self.calltip.keyPressEvent(event):
                return

        m = event.modifiers()
        k = event.key()
        t = event.text()
        # Disable some shortcuts
        if self.disable_shortcuts and \
                m & Qt.ControlModifier and k in [Qt.Key_A, Qt.Key_R,
                                                 Qt.Key_C, Qt.Key_K,
                                                 Qt.Key_X, Qt.Key_V,
                                                 Qt.Key_Y, Qt.Key_Z]:
            new_ev = QKeyEvent(event.type(), k, m, t)
            event.ignore()
            QCoreApplication.postEvent(self.parent(), new_ev)
            return
        elif k == Qt.Key_Tab:
            if self.tab_pressed():
                return
        elif k == Qt.Key_Backtab:
            if self.backtab_pressed():
                return
        elif k == Qt.Key_Backspace:
            if self.backspace_pressed():
                return
        elif k == Qt.Key_Period or \
                (k == Qt.Key_Space and event.modifiers() == Qt.ControlModifier):
            QPlainTextEdit.keyPressEvent(self, event)
            self.autocomplete_pressed()
            return
        elif k in [Qt.Key_ParenLeft, Qt.Key_BraceLeft, Qt.Key_BracketLeft]:
            QPlainTextEdit.keyPressEvent(self, event)
            self.paren_opened(k)
            return
        QPlainTextEdit.keyPressEvent(self, event)
        if k == Qt.Key_Return or k == Qt.Key_Enter:
            self.after_return_pressed()

    def paren_opened(self, key):
        close_char = {
            Qt.Key_ParenLeft: ")",
            Qt.Key_BraceLeft: " }",
            Qt.Key_BracketLeft: "]"
        }
        cursor = self.textCursor()
        cursor.insertText(close_char[key])
        cursor.setPosition(cursor.position() - 1)
        self.setTextCursor(cursor)


class EditorSidebar(QWidget):

    def __init__(self, editor):
        QWidget.__init__(self, editor)
        self.editor = editor
        self.view = editor.view
        self.doc = editor.view.document
        self.fm = self.fontMetrics()
        self.show_line_numbers = True

        self.setAutoFillBackground(True)
        # bg = editor.view.palette().base().color()
        # pal = QPalette()
        # pal.setColor(self.backgroundRole(), bg)
        # self.setPalette(pal)
        self.setBackgroundRole(QPalette.Base)

        self.doc().documentLayout().update.connect(self.update)
        self.view.verticalScrollBar().valueChanged.connect(self.update)
        self.first_row = self.last_row = self.rows = 0
        width = 10
        if self.show_line_numbers:
            width += self.fm.width("00000")
        self.setFixedWidth(width)

    def paintEvent(self, event):
        QWidget.paintEvent(self, event)
        p = QPainter(self)
        view = self.view
        first = view.firstVisibleBlock()
        first_row = first.blockNumber()
        block = first
        row = first_row
        y = view.contentOffset().y()
        pageBottom = max(
            view.height(),
            view.verticalScrollBar().value() + view.viewport().height())
        fm = self.fm
        w = self.width() - 8
        while block.isValid():
            txt = str(row).rjust(5)
            y = view.blockBoundingGeometry(block).y()
            if y >= pageBottom:
                break
            x = w - fm.width(txt)
            p.drawText(x, y, txt)
            row += 1
            block = block.next()
        p.end()


class EditorWidget(QFrame):

    def __init__(self, parent=None, text=None,
                 EditorSidebarClass=EditorSidebar,
                 EditorViewClass=EditorView):
        QFrame.__init__(self, parent)
        self.view = EditorViewClass(self, text)
        self.sidebar = EditorSidebarClass(self)
        self.setFrameStyle(QFrame.StyledPanel | QFrame.Sunken)
        self.setLineWidth(2)
        self.vlayout = QVBoxLayout()
        self.vlayout.setSpacing(0)
        self.setLayout(self.vlayout)
        self.hlayout = QHBoxLayout()
        self.vlayout.addLayout(self.hlayout)
        self.hlayout.addWidget(self.sidebar)
        self.hlayout.addWidget(self.view)
        self.vlayout.setContentsMargins(2, 2, 2, 2)

    def setPlainText(self, text):
        self.view.document().setPlainText(text)
        self.view.setModified(False)

    def isModified(self):
        return self.view.document().isModified()

    def toPlainText(self):
        return unicode(self.view.document().toPlainText())

    def setModified(self, flag):
        self.view.document().setModified(flag)


class PythonEditorWidget(EditorWidget):
    pass


class QtQmlEditorWidget(QPlainTextEdit):

    def __init__(self,  parent):
        QPlainTextEdit.__init__(self,  parent)
        self.highlighter = QtQmlHighlighter(self)


class SaveDialog(QMessageBox):

    def __init__(self, msg):
        QMessageBox.__init__(self)
        self.setWindowTitle("Save")
        self.setText(msg)
        self.setStandardButtons(QMessageBox.Save | QMessageBox.Discard | QMessageBox.Cancel)
        self.setDefaultButton(QMessageBox.Save)


if __name__ == "__main__":
    if __file__ == "<stdin>":
        __file__ = "./widget.py"
    import sys
    app = QApplication(sys.argv)
    src = open(__file__).read()
    edit = EditorWidget(text=src)
    edit.resize(640, 480)
    edit.show()
    app.exec_()
