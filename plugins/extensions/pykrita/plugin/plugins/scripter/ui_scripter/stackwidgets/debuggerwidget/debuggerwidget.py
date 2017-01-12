from PyQt5.QtWidgets import QWidget, QVBoxLayout, QToolBar, QTableWidget,QAction
from PyQt5.QtGui import QIcon
from . import stepaction, stopaction

class DebuggerWidget(QWidget):

    def __init__(self, scripter, parent=None):
        super(DebuggerWidget, self).__init__(parent)

        self.scripter = scripter
        self.setObjectName('Debugger')
        self.layout = QVBoxLayout()

        self.toolbar = QToolBar()
        self.stopAction = stopaction.StopAction(self.scripter, self)
        self.stepAction = stepaction.StepAction(self.scripter, self)
        self.toolbar.addAction(self.stopAction)
        self.toolbar.addAction(self.stepAction)
        self.disableToolbar(True)

        self.table = QTableWidget(4, 4)

        self.layout.addWidget(self.toolbar)
        self.layout.addWidget(self.table)
        self.setLayout(self.layout)

    def startDebugger(self):
        self.disableToolbar(False)

    def disableToolbar(self, status):
        for action in self.toolbar.actions():
            action.setDisabled(status)
