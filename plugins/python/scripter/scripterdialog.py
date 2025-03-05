"""
SPDX-FileCopyrightText: 2017 Eliakin Costa <eliakim170@gmail.com>

SPDX-License-Identifier: GPL-2.0-or-later
"""
try:
    from PyQt6.QtWidgets import QDialog
    from PyQt6.QtCore import Qt
except:
    from PyQt5.QtWidgets import QDialog
    from PyQt5.QtCore import Qt


class ScripterDialog(QDialog):

    def __init__(self, uicontroller, parent=None):
        super(ScripterDialog, self).__init__(parent)
        self.setAttribute(Qt.WidgetAttribute.WA_QuitOnClose, False)
        self.uicontroller = uicontroller

    def closeEvent(self, event):
        self.uicontroller._writeSettings()
        self.uicontroller._saveSettings()
        event.accept()
