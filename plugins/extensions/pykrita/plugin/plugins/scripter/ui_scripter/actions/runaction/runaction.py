from PyQt5.QtWidgets import QAction, QMessageBox
from PyQt5.QtGui import QIcon, QKeySequence
from PyQt5.QtCore import Qt
import sys
from . import docwrapper
import os
from scripter import resources_rc
import importlib


class RunAction(QAction):

    def __init__(self, scripter, parent=None):
        super(RunAction, self).__init__(parent)
        self.scripter = scripter

        self.editor = self.scripter.uicontroller.editor
        self.output = self.scripter.uicontroller.findTabWidget('OutPut', 'OutPutTextEdit')

        self.triggered.connect(self.run)

        self.setText('Run')
        self.setToolTip('Run Ctrl+R')
        self.setIcon(QIcon(':/icons/run.svg'))
        self.setShortcut(QKeySequence(Qt.CTRL + Qt.Key_R))

    @property
    def parent(self):
        return 'toolBar'

    def run(self):
        """ This method execute python code from an activeDocument (file) or direct
            from editor (ui_scripter/editor/pythoneditor.py). When executing code
            from a file, we use importlib to load this module/file and with
            "users_script" name. That's implementation seeks for a "main()" function in the script.
            When executing code from editor without creating a file, we compile
            this script to bytecode and we execute this in an empty scope. That's
            faster than use exec directly and cleaner, because we are using an empty scope. """

        self.scripter.uicontroller.setActiveWidget('OutPut')
        stdout = sys.stdout
        stderr = sys.stderr
        output = docwrapper.DocWrapper(self.output.document())
        output.write("======================================\n")
        sys.stdout = output
        sys.stderr = output
        script = self.editor.document().toPlainText()
        document = self.scripter.documentcontroller.activeDocument
        try:
            if document:
                spec = importlib.util.spec_from_file_location("users_script", document.filePath)
                users_module = importlib.util.module_from_spec(spec)
                spec.loader.exec_module(users_module)
                users_module.main()
            else:
                code = compile(script, '<string>', 'exec')
                exec(script, {})
        except Exception as e:
            self.scripter.uicontroller.showException(str(e))

        sys.stdout = stdout
        sys.stderr = stderr

        # scroll to bottom of output
        max = self.output.verticalScrollBar().maximum()
        self.output.verticalScrollBar().setValue(max)
