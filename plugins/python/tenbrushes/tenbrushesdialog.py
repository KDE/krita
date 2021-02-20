# SPDX-License-Identifier: CC0-1.0

from PyQt5.QtWidgets import QDialog


class TenBrushesDialog(QDialog):

    def __init__(self, uitenbrushes, parent=None):
        super(TenBrushesDialog, self).__init__(parent)

        self.uitenbrushes = uitenbrushes

    def accept(self):
        self.uitenbrushes.tenbrushes.writeSettings()

        super(TenBrushesDialog, self).accept()

    def closeEvent(self, event):
        event.accept()
