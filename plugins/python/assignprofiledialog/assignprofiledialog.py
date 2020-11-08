# SPDX-License-Identifier: CC0-1.0

from PyQt5.QtCore import Qt
from PyQt5.QtWidgets import (QDialogButtonBox, QDialog,
                             QMessageBox, QComboBox, QVBoxLayout)
from krita import Extension


class AssignProfileDialog(Extension):

    def __init__(self, parent):
        super(AssignProfileDialog, self).__init__(parent)

    def assignProfile(self):
        doc = Application.activeDocument()
        if doc is None:
            QMessageBox.information(
                Application.activeWindow().qwindow(),
                i18n("Assign Profile"),
                i18n("There is no active document."))
            return

        self.dialog = QDialog(Application.activeWindow().qwindow())

        self.cmbProfile = QComboBox(self.dialog)
        for profile in sorted(
                Application.profiles(doc.colorModel(), doc.colorDepth())):
            self.cmbProfile.addItem(profile)

        vbox = QVBoxLayout(self.dialog)
        vbox.addWidget(self.cmbProfile)
        self.buttonBox = QDialogButtonBox(self.dialog)
        self.buttonBox.setOrientation(Qt.Horizontal)
        self.buttonBox.setStandardButtons(
            QDialogButtonBox.Ok | QDialogButtonBox.Cancel)
        self.buttonBox.accepted.connect(self.dialog.accept)
        self.buttonBox.accepted.connect(self.accept)
        self.buttonBox.rejected.connect(self.dialog.reject)
        vbox.addWidget(self.buttonBox)
        self.dialog.show()
        self.dialog.activateWindow()
        self.dialog.exec_()

    def accept(self):
        doc = Application.activeDocument()
        doc.setColorProfile(self.cmbProfile.currentText())

    def setup(self):
        pass

    def createActions(self, window):
        action = window.createAction("assing_profile_to_image",
                                     i18n("Assign Profile to Image"))
        action.triggered.connect(self.assignProfile)
