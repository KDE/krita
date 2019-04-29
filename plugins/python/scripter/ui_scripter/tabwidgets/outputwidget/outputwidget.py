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
from PyQt5.QtWidgets import QWidget, QVBoxLayout, QToolBar, QTableWidget, QAction
from . import clearaction, outputtextedit


class OutPutWidget(QWidget):

    def __init__(self, scripter, parent=None):
        super(OutPutWidget, self).__init__(parent)

        self.scripter = scripter
        self.setObjectName(i18n('Output'))
        self.layout = QVBoxLayout()

        self.toolbar = QToolBar()
        self.clearAction = clearaction.ClearAction(self.scripter, self)
        self.toolbar.addAction(self.clearAction)

        self.outputtextedit = outputtextedit.OutPutTextEdit(self.scripter, self)

        self.layout.addWidget(self.toolbar)
        self.layout.addWidget(self.outputtextedit)
        self.setLayout(self.layout)
