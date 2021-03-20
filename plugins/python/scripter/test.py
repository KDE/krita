"""
SPDX-FileCopyrightText: 2017 Eliakin Costa <eliakim170@gmail.com>

SPDX-License-Identifier: GPL-2.0-or-later
"""
# editor.py

from PyQt5.QtCore import *
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *

import syntax

app = QApplication([])
editor = QPlainTextEdit()
f = QFont("monospace", 10, QFont.Normal)
f.setFixedPitch(True)
editor.document().setDefaultFont(f)
highlight = syntax.PythonHighlighter(editor.document())


editor.show()

# Load syntax.py into the editor for demo purposes

# infile = open('syntax.py', 'r')
# editor.setPlainText(infile.read())

app.exec_()
