# SPDX-License-Identifier: CC0-1.0

import krita
from PyQt5.QtGui import QPixmap, QIcon
from . import uitenbrushes


class TenBrushesExtension(krita.Extension):

    def __init__(self, parent):
        super(TenBrushesExtension, self).__init__(parent)

        self.actions = []
        self.buttons = []
        self.selectedPresets = []
        # Indicates whether we want to activate the previous-selected brush
        # on the second press of the shortcut
        self.activatePrev = True
        # Indicates whether we want to select the freehand brush tool
        # on the press of a preset shortcut
        self.autoBrush = False
        self.oldPreset = None

    def setup(self):
        self.readSettings()

    def createActions(self, window):
        action = window.createAction("ten_brushes", i18n("Ten Brushes"))
        action.setToolTip(i18n("Assign ten brush presets to ten shortcuts."))
        action.triggered.connect(self.initialize)
        self.loadActions(window)

    def initialize(self):
        self.uitenbrushes = uitenbrushes.UITenBrushes()
        self.uitenbrushes.initialize(self)

    def readSettings(self):
        self.selectedPresets = Application.readSetting("", "tenbrushes", "").split(',')

        setting = Application.readSetting("", "tenbrushesActivatePrev2ndPress", "True")
        # we should not get anything other than 'True' and 'False'
        self.activatePrev = setting == 'True'

        setting = Application.readSetting(
            "", "tenbrushesAutoBrushOnPress", "False")
        self.autoBrush = setting == 'True'

    def writeSettings(self):
        presets = []

        for index, button in enumerate(self.buttons):
            self.actions[index].preset = button.preset
            presets.append(button.preset)

        Application.writeSetting("", "tenbrushes", ','.join(map(str, presets)))
        Application.writeSetting("", "tenbrushesActivatePrev2ndPress",
                                 str(self.activatePrev))
        Application.writeSetting("", "tenbrushesAutoBrushOnPress",
                                 str(self.autoBrush))

    def loadActions(self, window):
        allPresets = Application.resources("preset")

        for index, item in enumerate(['1', '2', '3', '4', '5',
                                      '6', '7', '8', '9', '0']):
            action = window.createAction(
                "activate_preset_" + item,
                str(i18n("Activate Brush Preset {num}")).format(num=item), "")
            action.triggered.connect(self.activatePreset)

            if (index < len(self.selectedPresets)
                    and self.selectedPresets[index] in allPresets):
                action.preset = self.selectedPresets[index]
            else:
                action.preset = None

            self.actions.append(action)

    def activatePreset(self):
        allPresets = Application.resources("preset")
        window = Application.activeWindow()
        if (window and len(window.views()) > 0
                and self.sender().preset in allPresets):
            currentPreset = window.views()[0].currentBrushPreset()

            if self.autoBrush:
                Krita.instance().action('KritaShape/KisToolBrush').trigger()

            if (self.activatePrev
                    and self.sender().preset == currentPreset.name()):
                window.views()[0].activateResource(self.oldPreset)
            else:
                self.oldPreset = window.views()[0].currentBrushPreset()
                window.views()[0].activateResource(allPresets[self.sender().preset])

        preset = window.views()[0].currentBrushPreset()
        window.activeView().showFloatingMessage(str(i18n("{}\nselected")).format(preset.name()),
                                              QIcon(QPixmap.fromImage(preset.image())),
                                              1000, 1)

