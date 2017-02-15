import sys
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *
from krita import *

class DropButton(QPushButton):

    def __init__(self, parent):
        super().__init__(parent)
        self.setFixedSize(64, 64)
    

class TenBrushesViewExtension(ViewExtension):

    def __init__(self, parent):
        super().__init__(parent)

    def setup(self):
        action = Application.createAction("Ten Brushes")
        action.setToolTip("Assign ten brush presets to ten shortcuts.")
        action.triggered.connect(self.showDialog)
        
        # Read the ten selected brush presets from the settings
        selectedBrushes = Application.readSetting("", "tenbrushes", "")
        print(selectedBrushes.split(","))
        # Setup up to ten actions and give them default shortcuts
      
    def showDialog(self):
        self.dialog = QDialog(Application.activeWindow().qwindow())
        
        self.buttonBox = QDialogButtonBox(self.dialog)
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)
        self.buttonBox.accepted.connect(self.accept)
        self.buttonBox.rejected.connect(self.dialog.reject)
        
        vbox = QVBoxLayout(self.dialog)
        hbox = QHBoxLayout(self.dialog)
        for i in ['1', '2', '3', '4', '5', '6', '7', '8', '9', '0']:
            self.buttons = []
            button = DropButton(self.dialog)
            button.setText("Ctrl+" + i)
            hbox.addWidget(button)
            self.buttons.append(button)
        
        
        self.presetChooser = QTableWidget(self.dialog)
        self.presetChooser.setColumnCount(20)
        self.presetChooser.setRowCount(2000)
        self.presetChooser.setShowGrid(True);
        self.presetChooser.horizontalHeader().setVisible(False);
        self.presetChooser.verticalHeader().setVisible(False);
        self.presetChooser.setSortingEnabled(False)
        col = 0
        row = 0
        for preset in Application.resources("preset"):
            print(preset.name(), row, col)
            pm = QPixmap.fromImage(preset.image())
            icon = QIcon(pm)
            item = QTableWidgetItem(icon, preset.name())
            self.presetChooser.setItem(row, col, item)
            col = col + 1
            if col > self.presetChooser.columnCount():
                col = 0
                row = row + 1
        self.presetChooser.setRowCount(row)
        self.presetChooser.setSortingEnabled(True)
                
                                        
        vbox.addLayout(hbox)
        vbox.addWidget(self.presetChooser)
        vbox.addWidget(self.buttonBox)
        
        self.dialog.show()
        self.dialog.activateWindow()
        self.dialog.exec_()
        
        
    def accept(self):
        print("Saving ten presets")
        self.dialog.accept()

Scripter.addViewExtension(TenBrushesViewExtension(Application))
