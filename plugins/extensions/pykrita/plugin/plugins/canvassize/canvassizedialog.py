from PyQt5.QtWidgets import QDialog


class CanvasSizeDialog(QDialog):

    def __init__(self, parent=None):
        super(CanvasSizeDialog, self).__init__(parent)

    def closeEvent(self, event):
        event.accept()
