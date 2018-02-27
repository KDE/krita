'''
This script is licensed CC 0 1.0, so that you can learn from it.

------ CC 0 1.0 ---------------

The person who associated a work with this deed has dedicated the work to the public domain by waiving all of his or her rights to the work worldwide under copyright law, including all related and neighboring rights, to the extent allowed by law.

You can copy, modify, distribute and perform the work, even for commercial purposes, all without asking permission.

https://creativecommons.org/publicdomain/zero/1.0/legalcode
'''
import sys
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *
from krita import *


class AssignProfileDialog(Extension):

    def __init__(self, parent):
        super().__init__(parent)

    def assignProfile(self):
        doc = Application.activeDocument()
        if doc == None:
            QMessageBox.information(Application.activeWindow().qwindow(), "Assign Profile", "There is no active document.")
            return

        self.dialog = QDialog(Application.activeWindow().qwindow())

        self.cmbProfile = QComboBox(self.dialog)
        for profile in sorted(Application.profiles(doc.colorModel(), doc.colorDepth())):
            self.cmbProfile.addItem(profile)

        vbox = QVBoxLayout(self.dialog)
        vbox.addWidget(self.cmbProfile)
        self.buttonBox = QDialogButtonBox(self.dialog)
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)
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
        action = Application.createAction("assing_profile_to_image", "Assign Profile to Image")
        action.triggered.connect(self.assignProfile)

Scripter.addExtension(AssignProfileDialog(Application))
