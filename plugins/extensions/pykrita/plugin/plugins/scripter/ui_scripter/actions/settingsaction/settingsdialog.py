from PyQt5.QtWidgets import QDialog, QFormLayout
from . import syntaxstylescombobox, fontscombobox


class SettingsDialog(QDialog):

    def __init__(self, scripter, parent=None):
        super(SettingsDialog, self).__init__(parent)

        self.scripter = scripter
        self.setWindowTitle('Settings')
        self.mainLayout = QFormLayout(self)
        self.mainLayout.addRow('Syntax Highlither', syntaxstylescombobox.SyntaxStylesComboBox(self.scripter.uicontroller.highlight))
        self.mainLayout.addRow('Fonts', fontscombobox.FontsComboBox(self.scripter.uicontroller.editor))

    def readSettings(self, settings):
        for index in range(self.mainLayout.rowCount()):
            widget = self.mainLayout.itemAt(index, QFormLayout.FieldRole).widget()
            widget.readSettings(settings)

    def writeSettings(self, settings):
        for index in range(self.mainLayout.rowCount()):
            widget = self.mainLayout.itemAt(index, QFormLayout.FieldRole).widget()
            widget.writeSettings(settings)
