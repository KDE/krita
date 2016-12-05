from PyQt5.QtWidgets import QAction, QMessageBox
from PyQt5.QtGui import QIcon
import sys
from ..editor import docwrapper

class RunAction(QAction):

    def __init__(self, scripter, parent=None):
        super(RunAction, self).__init__(parent)
        self.scripter = scripter

        self.editor = self.scripter.uicontroller.editor
        self.output = self.scripter.uicontroller.output

        self.triggered.connect(self.run)

        self.setText('Run')
        self.setIcon(QIcon('/home/eliakincosta/Pictures/play.svg'))

    @property
    def parent(self):
        return 'toolBar'

    def run(self):
        stdout = sys.stdout
        stderr = sys.stderr
        output = docwrapper.DocWrapper(self.output.document())
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
