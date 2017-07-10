from PyQt5.QtWidgets import QDialog


class ExportLayersDialog(QDialog):

    def __init__(self, parent=None):
        super(ExportLayersDialog, self).__init__(parent)

    def closeEvent(self, event):
        event.accept()
