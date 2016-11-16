# -*- coding: utf-8 -*-

from PyQt5.QtWidgets import *
from PyQt5.QtCore import *

class LineNumberArea(QWidget):

    def __init__(self, editor):
        super(LineNumberArea, self).__init__(editor)
        self.codeEditor = editor

    def sizeHint(self):
        return QSize(self.codeEditor.lineNumberAreaWidth(), 0)

    def paintEvent(self, event):
        """It Invokes the draw method(lineNumberAreaPaintEvent) in CodeEditor"""
        self.codeEditor.lineNumberAreaPaintEvent(event)
