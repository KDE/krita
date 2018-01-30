from PyQt5.QtWidgets import QPlainTextEdit


class OutPutTextEdit(QPlainTextEdit):

    def __init__(self, scripter, toolbar, parent=None):
        super(OutPutTextEdit, self).__init__(parent)

        self.setObjectName('OutPutTextEdit')
        self.setReadOnly(True)
        doc = self.document()
        font = doc.defaultFont()
        font.setFamily('Monospace')
        doc.setDefaultFont(font)
