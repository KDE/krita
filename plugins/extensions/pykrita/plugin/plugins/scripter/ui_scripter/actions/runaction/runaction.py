from PyQt5.QtWidgets import QAction, QMessageBox
from PyQt5.QtGui import QIcon
import sys
from . import docwrapper
import os


class RunAction(QAction):

    def __init__(self, scripter, parent=None):
        super(RunAction, self).__init__(parent)
        self.scripter = scripter

        self.editor = self.scripter.uicontroller.editor
        self.output = self.scripter.uicontroller.findStackWidget('OutPut')

        self.triggered.connect(self.run)

        self.setText('Run')
        # path to the icon
        #self.setIcon(QIcon('/home/eliakincosta/Pictures/play.svg'))

    @property
    def parent(self):
        return 'toolBar'

    def run(self):
        document = self.scripter.uicontroller.invokeAction('save')

        if document:
            stdout = sys.stdout
            stderr = sys.stderr
            output = docwrapper.DocWrapper(self.output.document())
            output.write("======================================\n")
            sys.stdout = output
            sys.stderr = output
            script = self.editor.document().toPlainText()
            try:
                bc = compile(document.data, document.filePath, "exec")
                exec(bc)
            except Exception as e:
                self.scripter.uicontroller.showException(str(e))
            sys.stdout = stdout
            sys.stderr = stderr
