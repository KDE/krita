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
        self.actions = []
        self.buttons = []

        self.buttonBox.accepted.connect(self.mainDialog.accept)
        self.buttonBox.rejected.connect(self.mainDialog.reject)

        self.buttonBox.setOrientation(Qt.Horizontal)
        self.buttonBox.setStandardButtons(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)

        self.presetChooser = krita.PresetChooser(self.mainDialog)

    def initialize(self):
        self.readSettings()

        allPresets = Application.resources("preset")

        for index, item in enumerate(['1', '2', '3', '4', '5', '6', '7', '8', '9', '0']):
            buttonLayout = QVBoxLayout()
            button = dropbutton.DropButton(self.mainDialog)
            button.setObjectName(item)
            button.clicked.connect(button.selectPreset)
            button.presetChooser = self.presetChooser

            if self.actions[index] and self.actions[index].preset and self.actions[index].preset in allPresets:
                p = allPresets[self.actions[index].preset];
                button.preset = p.name()
                button.setIcon(QIcon(QPixmap.fromImage(p.image())))

            buttonLayout.addWidget(button)
            label = QLabel("Ctrl+Alt+" + item)
            label.setAlignment(Qt.AlignHCenter)
            buttonLayout.addWidget(label)

            self.hbox.addLayout(buttonLayout)
            self.buttons.append(button)

        self.vbox.addLayout(self.hbox)
        self.vbox.addWidget(self.presetChooser)
        self.vbox.addWidget(self.buttonBox)
        self.vbox.addWidget(QLabel("Select the brush preset, then click on the button you want to use to select the preset"))

        self.mainDialog.show()
        self.mainDialog.activateWindow()
        self.mainDialog.exec_()

    def activatePreset(self):
        allPresets = self.kritaInstance.resources("preset")
        print("activatePreset", self.sender().preset)
        if Application.activeWindow() and len(Application.activeWindow().views()) > 0 and self.sender().preset in allPresets:
            Application.activeWindow().views()[0].activateResource(allPresets[self.sender().preset])

    def readSettings(self):
        # Read the ten selected brush presets from the settings
        # That part can be a loadPresets method 43 - 58, but it really needs a refactoring
        selectedPresets = self.kritaInstance.readSetting("", "tenbrushes", "").split(',')
        allPresets = self.kritaInstance.resources("preset")

        # Setup up to ten actions and give them default shortcuts
        for index, item in enumerate(['1', '2', '3', '4', '5', '6', '7', '8', '9', '0']):
            action = self.kritaInstance.createAction("activate_preset_" + item, "Activate Preset " + item)
            #action.setVisible(False)
            action.setMenu("None")
            action.triggered.connect(self.activatePreset)
            if index < len(selectedPresets) and selectedPresets[index] in allPresets:
                action.preset = selectedPresets[index]
            else:
                action.preset = None
            self.actions.append(action)

    def writeSettings(self):
        i = 0
        presets = []
        for button in self.buttons:
            self.actions[i].preset = button.preset
            presets.append(button.preset)
            i = i + 1
        self.kritaInstance.writeSetting("", "tenbrushes", ','.join(map(str, presets)))
