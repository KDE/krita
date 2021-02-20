# SPDX-License-Identifier: CC0-1.0

from PyQt5.QtWidgets import QDialog


class ColorSpaceDialog(QDialog):

    def __init__(self, parent=None):
        super(ColorSpaceDialog, self).__init__(parent)

    def closeEvent(self, event):
        event.accept()
