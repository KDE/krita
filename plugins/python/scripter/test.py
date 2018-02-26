"""
Copyright (c) 2017 Eliakin Costa <eliakim170@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
