import sys
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *
from krita import *

class DropButton(QPushButton):

    def __init__(self, parent):
        super().__init__(parent)
        self.setFixedSize(64, 64)
        self.setIconSize(QSize(64, 64))
        self.preset = None
  
    def selectPreset(self):
        self.preset = self.presetChooser.currentPreset().name()
        self.setIcon(QIcon(QPixmap.fromImage(self.presetChooser.currentPreset().image())))
        

class TenBrushesExtension(Extension):

    def __init__(self, parent):
        super().__init__(parent)
        self.buttons = []
        self.actions = []
        
    def setup(self):
        action = Application.createAction("Ten Brushes")
        action.setToolTip("Assign ten brush presets to ten shortcuts.")
        action.triggered.connect(self.showDialog)
        
        # Read the ten selected brush presets from the settings
        selectedPresets = Application.readSetting("", "tenbrushes", "").split(',')
        allPresets = Application.resources("preset")
        # Setup up to ten actions and give them default shortcuts
        j = 0
        self.actions = []
        for i in ['1', '2', '3', '4', '5', '6', '7', '8', '9', '0']:
            action = Application.createAction("Activate Preset " + i)
            action.setVisible(False)
            action.setShortcut("CTRL+" + i)
            action.triggered.connect(self.activatePreset)
            if j < len(selectedPresets) and selectedPresets[j] in allPresets:
                action.preset = selectedPresets[j]
            else:
                action.preset = None
            self.actions.append(action)
            j = j + 1

      
    def activatePreset(self):
        allPresets = Application.resources("preset")
        if Application.activeWindow() and len(Application.activeWindow().views()) > 0 and self.sender().preset in allPresets:
            Application.activeWindow().views()[0].activateResource(allPresets[self.sender().preset])
      
    def showDialog(self):
        self.dialog = QDialog(Application.activeWindow().qwindow())
        
        self.buttonBox = QDialogButtonBox(self.dialog)
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)
        self.buttonBox.accepted.connect(self.accept)
        self.buttonBox.rejected.connect(self.dialog.reject)
        
        vbox = QVBoxLayout(self.dialog)
        hbox = QHBoxLayout(self.dialog)
        
        self.presetChooser = PresetChooser(self.dialog)
        
        allPresets = Application.resources("preset")
        j = 0
        self.buttons = []        
        for i in ['1', '2', '3', '4', '5', '6', '7', '8', '9', '0']:
            buttonBox = QVBoxLayout()
            button = DropButton(self.dialog)
            button.setObjectName(i)
            button.clicked.connect(button.selectPreset)
            button.presetChooser = self.presetChooser
            
            if self.actions[j] and self.actions[j].preset and self.actions[j].preset in allPresets:
                p = allPresets[self.actions[j].preset];
                button.preset = p.name()
                button.setIcon(QIcon(QPixmap.fromImage(p.image())))
                        
            buttonBox.addWidget(button)
            label = QLabel("Ctrl+" + i)
            label.setAlignment(Qt.AlignHCenter)
            buttonBox.addWidget(label)
            hbox.addLayout(buttonBox)
            self.buttons.append(button)
            j = j + 1
                                        
        vbox.addLayout(hbox)
        vbox.addWidget(self.presetChooser)
        vbox.addWidget(self.buttonBox)
        vbox.addWidget(QLabel("Select the brush preset, then click on the button you want to use to select the preset"))
        
        self.dialog.show()
        self.dialog.activateWindow()
        self.dialog.exec_()
        
        
    def accept(self):
        i = 0
        presets = []
        for button in self.buttons:
            self.actions[i].preset = button.preset
            presets.append(button.preset)
            i = i + 1
        Application.writeSetting("", "tenbrushes", ','.join(map(str, presets)))
        self.dialog.accept()

Scripter.addExtension(TenBrushesExtension(Application))
