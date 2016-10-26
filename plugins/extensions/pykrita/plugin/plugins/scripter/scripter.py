from PyQt5.QtGui import *
from PyQt5.QtWidgets import *
from krita import *
from scripter import syntax
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

    def showScripter(self):
        dialog = QDialog()
        dialog.setWindowModality(Qt.NonModal)
        self.editor = QPlainTextEdit()
        f = QFont("monospace", 10, QFont.Normal)
        f.setFixedPitch(True)
        self.editor.document().setDefaultFont(f)
        highlight = syntax.PythonHighlighter(self.editor.document())
        vbox = QVBoxLayout(dialog)
        vbox.addWidget(self.editor)
        button = QPushButton("Execute")
        button.clicked.connect(self.execute)
        vbox.addWidget(button)
        self.output = QPlainTextEdit()
        vbox.addWidget(self.output)
        dialog.resize(400, 500)
        dialog.setWindowTitle("Scripter")
        dialog.setSizeGripEnabled(True)
        dialog.show()
        dialog.activateWindow()
        dialog.exec()


Krita.instance().addViewExtension(ScripterViewExtension(Krita.instance()))

