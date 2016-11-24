from PyQt5.QtGui import *
from PyQt5.QtWidgets import *
from PyQt5.QtCore import *
from krita import *
from scripter import syntax, pythoneditor, settingsdialog, syntaxstyles
import sys

class docWrapper:

    def __init__(self, textdocument):
        self.textdocument = textdocument

    def write(self, text, view = None):
        cursor = QTextCursor(self.textdocument)
        cursor.clearSelection()
        cursor.movePosition(QTextCursor.End)
        cursor.insertText(text)

class ScripterViewExtension(ViewExtension):

    def __init__(self, parent):
        super().__init__(parent)

    def setup(self):
        print("Scripter setup")
        action = Krita.instance().createAction("Scripter")
        action.triggered.connect(self.showScripter)

    def execute(self):
        stdout = sys.stdout
        stderr = sys.stderr
        output = docWrapper(self.output.document())
        output.write("======================================\n")
        sys.stdout = output
        sys.stderr = output
        script = self.editor.document().toPlainText()
        try:
            bc = compile(script, "<string>", "exec")
        except Exception as e:
            QMessageBox.critical(self.editor, "Error compiling script", str(e))
            return

        exec(bc)

        sys.stdout = stdout
        sys.stderr = stderr

    def openSettings(self):
        self.settingsDialog = settingsdialog.SettingsDialog(self)
        self.settingsDialog.setWindowModality(Qt.WindowModal)
        self.settingsDialog.setFixedSize(400, 250)
        self.settingsDialog.show()
        self.settingsDialog.exec()

    def showScripter(self):
        dialog = QDialog()
        dialog.setWindowModality(Qt.NonModal)
        self.editor = pythoneditor.CodeEditor()
        self.highlight = syntax.PythonHighlighter(self.editor.document(), syntaxstyles.DefaultSyntaxStyle())
        vbox = QVBoxLayout(dialog)
        vbox.addWidget(self.editor)
        buttonLayout = QHBoxLayout()
        button = QPushButton("Run")
        settingsButton = QPushButton("Settings")
        button.clicked.connect(self.execute)
        settingsButton.clicked.connect(self.openSettings)
        buttonLayout.addWidget(button)
        buttonLayout.addWidget(settingsButton)
        vbox.addLayout(buttonLayout)
        self.output = QPlainTextEdit()
        vbox.addWidget(self.output)
        dialog.resize(400, 500)
        dialog.setWindowTitle("Scripter")
        dialog.setSizeGripEnabled(True)
        dialog.show()
        dialog.activateWindow()
        dialog.exec()


Krita.instance().addViewExtension(ScripterViewExtension(Krita.instance()))
