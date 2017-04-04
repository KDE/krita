from PyQt5.QtWidgets import QAction
from PyQt5.QtCore import Qt
from . import settingsdialog


class SettingsAction(QAction):

    def __init__(self, scripter, parent=None):
        super(SettingsAction, self).__init__(parent)
        self.scripter = scripter

        self.triggered.connect(self.openSettings)

        self.settingsDialog = settingsdialog.SettingsDialog(self.scripter)
        self.settingsDialog.setWindowModality(Qt.WindowModal)
        self.settingsDialog.setFixedSize(400, 250)

        self.setText('Settings')

    @property
    def parent(self):
        return 'File'

    def openSettings(self):
        self.settingsDialog.show()
        self.settingsDialog.exec()

    def readSettings(self):
        self.settingsDialog.readSettings(self.scripter.settings)

    def writeSettings(self):
        self.settingsDialog.writeSettings(self.scripter.settings)
