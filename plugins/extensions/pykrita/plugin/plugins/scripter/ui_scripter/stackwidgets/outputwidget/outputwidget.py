from PyQt5.QtWidgets import QPlainTextEdit


class OutPutWidget(QPlainTextEdit):

    def __init__(self, scripter, parent=None):
        super(OutPutWidget, self).__init__(parent)

        self.scripter = scripter
        self.setObjectName('OutPut')
