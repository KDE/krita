# SPDX-License-Identifier: CC0-1.0

try:
    from PyQt6.QtWidgets import QDialog
except:
    from PyQt5.QtWidgets import QDialog


class TenScriptsDialog(QDialog):

    def __init__(self, uitenscripts, parent=None):
        super(TenScriptsDialog, self).__init__(parent)

        self.uitenscripts = uitenscripts

    def accept(self):
        self.uitenscripts.tenscripts.writeSettings()

        super(TenScriptsDialog, self).accept()

    def closeEvent(self, event):
        event.accept()
