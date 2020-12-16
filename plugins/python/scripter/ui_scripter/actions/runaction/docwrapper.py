"""
SPDX-FileCopyrightText: 2017 Eliakin Costa <eliakim170@gmail.com>

SPDX-License-Identifier: GPL-2.0-or-later
"""
from PyQt5.QtGui import QTextCursor


class DocWrapper(object):

    def __init__(self, textdocument):
        self.textdocument = textdocument

    def write(self, text, view=None):
        cursor = QTextCursor(self.textdocument)
        cursor.clearSelection()
        cursor.movePosition(QTextCursor.End)
        cursor.insertText(text)
