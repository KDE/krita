import krita
from tenbrushes import uitenbrushes


class TenBrushesExtension(krita.Extension):

    def __init__(self, parent):
        super(TenBrushesExtension, self).__init__(parent)

        self.actions = []
        self.buttons = []
        self.selectedPresets = []

    def setup(self):
        action = Application.createAction("ten_brushes", "Ten Brushes")
        action.setToolTip("Assign ten brush presets to ten shortcuts.")
        action.triggered.connect(self.initialize)

        self.readSettings()
        self.loadActions()

    def initialize(self):
        self.uitenbrushes = uitenbrushes.UITenBrushes()
        self.uitenbrushes.initialize(self)

    def readSettings(self):
        self.selectedPresets = Application.readSetting("", "tenbrushes", "").split(',')

    def writeSettings(self):
        presets = []

        for index, button in enumerate(self.buttons):
            self.actions[index].preset = button.preset
            presets.append(button.preset)
        Application.writeSetting("", "tenbrushes", ','.join(map(str, presets)))

    def loadActions(self):
        allPresets = Application.resources("preset")

        for index, item in enumerate(['1', '2', '3', '4', '5', '6', '7', '8', '9', '0']):
            action = Application.createAction("activate_preset_" + item, "Activate Brush Preset " + item)
            action.setMenu("None")
            action.triggered.connect(self.activatePreset)

            if index < len(self.selectedPresets) and self.selectedPresets[index] in allPresets:
                action.preset = self.selectedPresets[index]
            else:
                action.preset = None

            self.actions.append(action)

    def activatePreset(self):
        allPresets = Application.resources("preset")
        if Application.activeWindow() and len(Application.activeWindow().views()) > 0 and self.sender().preset in allPresets:
            Application.activeWindow().views()[0].activateResource(allPresets[self.sender().preset])


Scripter.addExtension(TenBrushesExtension(Application))
