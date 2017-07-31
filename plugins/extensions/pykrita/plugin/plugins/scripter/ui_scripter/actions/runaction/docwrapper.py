from PyQt5.QtGui import QTextCursor


class DocWrapper:

    def __init__(self, textdocument):
        self.textdocument = textdocument

    def write(self, text, view=None):
        cursor = QTextCursor(self.textdocument)
        cursor.clearSelection()
        cursor.movePosition(QTextCursor.End)
        cursor.insertText(text)
