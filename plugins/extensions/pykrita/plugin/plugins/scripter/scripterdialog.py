from PyQt5.QtWidgets import QDialog


class ScripterDialog(QDialog):

    def __init__(self, uicontroller, parent=None):
        super(ScripterDialog, self).__init__(parent)

        self.uicontroller = uicontroller

    def closeEvent(self, event):
        self.uicontroller._writeSettings()
        self.uicontroller._saveSettings()
        event.accept()
