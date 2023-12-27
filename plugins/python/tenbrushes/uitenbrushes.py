# SPDX-License-Identifier: CC0-1.0

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

        self.checkBoxActivatePrev = QCheckBox(
            i18n("&Activate previous brush preset when pressing the shortcut for the "
                 "second time"),
            self.mainDialog)

        self.checkBoxAutoBrush = QCheckBox(
            i18n("&Select freehand brush tool when pressing a shortcut"),
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
                        "you want to use to select the preset.")))
        self.vbox.addWidget(
            QLabel(i18n("Shortcuts are configurable through the <i>Keyboard Shortcuts</i> "
                        "interface in Krita's settings.")))

        self.vbox.addWidget(self.presetChooser)

        self.checkBoxActivatePrev.setChecked(self.tenbrushes.activatePrev)
        self.checkBoxActivatePrev.toggled.connect(self.setActivatePrev)
        self.vbox.addWidget(self.checkBoxActivatePrev)

        self.checkBoxAutoBrush.setChecked(self.tenbrushes.autoBrush)
        self.checkBoxAutoBrush.toggled.connect(self.setAutoBrush)
        self.vbox.addWidget(self.checkBoxAutoBrush)

        self.vbox.addWidget(self.buttonBox)

        self.mainDialog.show()
        self.mainDialog.activateWindow()
        self.mainDialog.exec_()

    def setActivatePrev(self, checked):
        self.tenbrushes.activatePrev = checked

    def setAutoBrush(self, checked):
        self.tenbrushes.autoBrush = checked

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

            action = Application.action("activate_preset_" + item)

            preset = self.tenbrushes.selectedPresets[index] if index < len(self.tenbrushes.selectedPresets) else None

            if action and preset and preset in allPresets:
                p = allPresets[preset]
                button.preset = p.name()
                button.setIcon(QIcon(QPixmap.fromImage(p.image())))

            buttonLayout.addWidget(button)

            label = QLabel(
                action.shortcut().toString())
            label.setAlignment(Qt.AlignHCenter)
            buttonLayout.addWidget(label)

            self.hbox.addLayout(buttonLayout)
            self.tenbrushes.buttons.append(button)
