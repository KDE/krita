# SPDX-License-Identifier: CC0-1.0

from PyQt5.QtWidgets import QDialog


class FilterManagerDialog(QDialog):

    def __init__(self, parent=None):
        super(FilterManagerDialog, self).__init__(parent)

    def closeEvent(self, event):
        event.accept()
