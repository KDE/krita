# SPDX-License-Identifier: CC0-1.0

import krita
from PyQt5.QtGui import QPixmap, QIcon
from . import uitenbrushes


class TenBrushesExtension(krita.Extension):

    def __init__(self, parent):
        super(TenBrushesExtension, self).__init__(parent)

        self.buttons = []
        self.selectedPresets = []
        self.actionToIndex = {}
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
        allPresets = Application.resources("preset")

        self.selectedPresets = Application.readSetting("", "tenbrushes", "").split(',')

        # in Krita 4.x we used to replace spaces in preset names with
        # underscores, which has changed in Krita 5.x. Here we just
        # try hard to load the legacy preset

        for index, preset in enumerate(self.selectedPresets):
            for name in [preset, preset.replace('_', ' ')]:
                if name in allPresets:
                    if name != preset:
                        self.selectedPresets[index] = name
                    break

        setting = Application.readSetting("", "tenbrushesActivatePrev2ndPress", "True")
        # we should not get anything other than 'True' and 'False'
        self.activatePrev = setting == 'True'

        setting = Application.readSetting(
            "", "tenbrushesAutoBrushOnPress", "False")
        self.autoBrush = setting == 'True'

    def writeSettings(self):
        presets = []

        for index, button in enumerate(self.buttons):
            presets.append(button.preset)
            self.selectedPresets.insert(index, button.preset)

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
            self.actionToIndex[action.objectName()] = index;

    def activatePreset(self):
        allPresets = Application.resources("preset")
        window = Application.activeWindow()

        presetIndex = self.actionToIndex[self.sender().objectName()]
        preset = self.selectedPresets[presetIndex] if len(self.selectedPresets) > presetIndex else None

        if (window and len(window.views()) > 0
                and preset
                and preset in allPresets):
            currentPreset = window.views()[0].currentBrushPreset()

            if self.autoBrush:
                Krita.instance().action('KritaShape/KisToolBrush').trigger()

            if (self.activatePrev
                    and preset == currentPreset.name()):
                window.views()[0].activateResource(self.oldPreset)
            else:
                self.oldPreset = window.views()[0].currentBrushPreset()
                window.views()[0].activateResource(allPresets[preset])

        preset = window.views()[0].currentBrushPreset()
        window.activeView().showFloatingMessage(str(i18n("{}\nselected")).format(preset.name()),
                                              QIcon(QPixmap.fromImage(preset.image())),
                                              1000, 1)

