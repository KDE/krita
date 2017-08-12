from PyQt5.QtWidgets import QDialog


class DocumentToolsDialog(QDialog):

    def __init__(self, parent=None):
        super(DocumentToolsDialog, self).__init__(parent)

    def closeEvent(self, event):
        event.accept()
