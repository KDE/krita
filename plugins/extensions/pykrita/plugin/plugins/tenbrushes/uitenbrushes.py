from PyQt5.QtCore import Qt, QSize
from PyQt5.QtGui import QPixmap, QIcon
from PyQt5.QtWidgets import (QDialogButtonBox, QLabel, QVBoxLayout, QHBoxLayout)
from tenbrushes import tenbrushesdialog, dropbutton
import krita


class UITenBrushes(object):

    def __init__(self):
        self.kritaInstance = krita.Krita.instance()
        self.mainDialog = tenbrushesdialog.TenBrushesDialog(self, self.kritaInstance.activeWindow().qwindow())

        self.buttonBox = QDialogButtonBox(self.mainDialog)
        self.vbox = QVBoxLayout(self.mainDialog)
        self.hbox = QHBoxLayout(self.mainDialog)

        self.buttonBox.accepted.connect(self.mainDialog.accept)
        self.buttonBox.rejected.connect(self.mainDialog.reject)

        self.buttonBox.setOrientation(Qt.Horizontal)
        self.buttonBox.setStandardButtons(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)

        self.presetChooser = krita.PresetChooser(self.mainDialog)

    def initialize(self, tentbrushes):
        self.tentbrushes = tentbrushes

        self.loadButtons()

        self.vbox.addLayout(self.hbox)
        self.vbox.addWidget(self.presetChooser)
        self.vbox.addWidget(self.buttonBox)
        self.vbox.addWidget(QLabel("Select the brush preset, then click on the button you want to use to select the preset"))

        self.mainDialog.show()
        self.mainDialog.activateWindow()
        self.mainDialog.exec_()

    def loadButtons(self):
        self.tentbrushes.buttons = []

        allPresets = Application.resources("preset")

        for index, item in enumerate(['1', '2', '3', '4', '5', '6', '7', '8', '9', '0']):
            buttonLayout = QVBoxLayout()
            button = dropbutton.DropButton(self.mainDialog)
            button.setObjectName(item)
            button.clicked.connect(button.selectPreset)
            button.presetChooser = self.presetChooser

            if self.tentbrushes.actions[index] and self.tentbrushes.actions[index].preset and self.tentbrushes.actions[index].preset in allPresets:
                p = allPresets[self.tentbrushes.actions[index].preset]
                button.preset = p.name()
                button.setIcon(QIcon(QPixmap.fromImage(p.image())))

            buttonLayout.addWidget(button)
            label = QLabel("Ctrl+Alt+" + item)
            label.setAlignment(Qt.AlignHCenter)
            buttonLayout.addWidget(label)

            self.hbox.addLayout(buttonLayout)
            self.tentbrushes.buttons.append(button)
