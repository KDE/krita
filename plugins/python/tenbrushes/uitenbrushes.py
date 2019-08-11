# This script is licensed CC 0 1.0, so that you can learn from it.

# ------ CC 0 1.0 ---------------

# The person who associated a work with this deed has dedicated the
# work to the public domain by waiving all of his or her rights to the
# work worldwide under copyright law, including all related and
# neighboring rights, to the extent allowed by law.

# You can copy, modify, distribute and perform the work, even for
# commercial purposes, all without asking permission.

# https://creativecommons.org/publicdomain/zero/1.0/legalcode

from PyQt5.QtCore import Qt
from PyQt5.QtGui import QPixmap, QIcon
from PyQt5.QtWidgets import (QDialogButtonBox, QLabel, QVBoxLayout,
                             QHBoxLayout, QCheckBox)
from . import tenbrushesdialog, dropbutton
import krita


class UITenBrushes(object):

    def __init__(self):
        self.kritaInstance = krita.Krita.instance()
        self.mainDialog = tenbrushesdialog.TenBrushesDialog(
            self, self.kritaInstance.activeWindow().qwindow())

        self.buttonBox = QDialogButtonBox(self.mainDialog)
        self.vbox = QVBoxLayout(self.mainDialog)
        self.hbox = QHBoxLayout(self.mainDialog)
        self.checkBox = QCheckBox(
            i18n("&Activate previous brush when pressing the shortcut for the "
                 "second time"),
            self.mainDialog)

        self.buttonBox.accepted.connect(self.mainDialog.accept)
        self.buttonBox.rejected.connect(self.mainDialog.reject)

        self.buttonBox.setOrientation(Qt.Horizontal)
        self.buttonBox.setStandardButtons(
            QDialogButtonBox.Ok | QDialogButtonBox.Cancel)

        self.presetChooser = krita.PresetChooser(self.mainDialog)

    def initialize(self, tenbrushes):
        self.tenbrushes = tenbrushes

        self.loadButtons()

        self.vbox.addLayout(self.hbox)
        self.vbox.addWidget(
            QLabel(i18n("Select the brush preset, then click on the button "
                        "you want to use to select the preset")))
        self.vbox.addWidget(self.presetChooser)

        self.checkBox.setChecked(self.tenbrushes.activatePrev)
        self.checkBox.toggled.connect(self.setActivatePrev)
        self.vbox.addWidget(self.checkBox)

        self.vbox.addWidget(self.buttonBox)

        self.mainDialog.show()
        self.mainDialog.activateWindow()
        self.mainDialog.exec_()

    def setActivatePrev(self, checked):
        self.tenbrushes.activatePrev = checked

    def loadButtons(self):
        self.tenbrushes.buttons = []

        allPresets = Application.resources("preset")

        for index, item in enumerate(['1', '2', '3', '4', '5',
                                      '6', '7', '8', '9', '0']):
            buttonLayout = QVBoxLayout()
            button = dropbutton.DropButton(self.mainDialog)
            button.setObjectName(item)
            button.clicked.connect(button.selectPreset)
            button.presetChooser = self.presetChooser

            action = self.tenbrushes.actions[index]

            if action and action.preset and action.preset in allPresets:
                p = allPresets[action.preset]
                button.preset = p.name()
                button.setIcon(QIcon(QPixmap.fromImage(p.image())))

            buttonLayout.addWidget(button)

            label = QLabel(
                action.shortcut().toString())
            label.setAlignment(Qt.AlignHCenter)
            buttonLayout.addWidget(label)

            self.hbox.addLayout(buttonLayout)
            self.tenbrushes.buttons.append(button)
